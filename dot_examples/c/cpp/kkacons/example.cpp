#include <boost/asio.hpp>
#include <librdkafka/rdkafkacpp.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

struct SomeEvent {};

struct ActionsEvent {
	std::vector<SomeEvent> actions;
};

struct ConnErrorEvent {
	std::string msg;
};

using KafkaEvent = std::variant<ActionsEvent, ConnErrorEvent>;
using KafkaSink = std::function<void(KafkaEvent &&)>;

class KafkaConsumer : public std::enable_shared_from_this<KafkaConsumer> {
    public:
	struct Config {
		std::string brokers;
		std::string group_id;
		std::string topic;
		std::chrono::milliseconds poll_interval{ 100 };
		int max_messages_per_poll{ 256 };
		std::chrono::milliseconds error_emit_min_interval{ 5000 };
	};

	KafkaConsumer(boost::asio::io_service &ios, Config cfg, KafkaSink sink)
		: strand_(ios)
		, timer_(ios)
		, cfg_(std::move(cfg))
		, sink_(std::move(sink))
	{
	}

	~KafkaConsumer()
	{
		stop();
	}

	std::string start()
	{
		if (running_)
			return {};

		std::string err = init_consumer();
		if (!err.empty())
			return err;

		running_ = true;
		schedule_poll();
		return {};
	}

	void stop()
	{
		auto wk = weak_from_this();
		strand_.post([wk]() {
			if (auto self = wk.lock())
				self->stop_on_strand();
		});
	}

    private:
	boost::asio::io_service::strand strand_;
	boost::asio::steady_timer timer_;
	Config cfg_;
	KafkaSink sink_;

	bool running_ = false;

	std::unique_ptr<RdKafka::KafkaConsumer> consumer_;

	std::string last_err_msg_;
	std::chrono::steady_clock::time_point last_err_emit_{};

	class RebalanceCb : public RdKafka::RebalanceCb {
	    public:
		explicit RebalanceCb(KafkaConsumer *owner)
			: owner_(owner)
		{
		}

		void rebalance_cb(RdKafka::KafkaConsumer *consumer, RdKafka::ErrorCode err,
				  std::vector<RdKafka::TopicPartition *> &partitions) override
		{
			if (!owner_)
				return;

			if (err == RdKafka::ERR__ASSIGN_PARTITIONS) {
				consumer->assign(partitions);
				return;
			}

			if (err == RdKafka::ERR__REVOKE_PARTITIONS) {
				consumer->unassign();
				return;
			}

			owner_->emit_error_throttled("rebalance: " + RdKafka::err2str(err));
		}

	    private:
		KafkaConsumer *owner_;
	};

	RebalanceCb rebalance_cb_{ this };

	static bool parse_actions_append(const void *data, std::size_t len,
					 std::vector<SomeEvent> &out, std::string &err)
	{
		(void)data;
		(void)len;
		err.clear();
		return true;
	}

	void stop_on_strand()
	{
		if (!running_)
			return;

		running_ = false;

		boost::system::error_code ignored;
		timer_.cancel(ignored);

		if (consumer_) {
			consumer_->close();
			consumer_.reset();
		}
	}

	std::string init_consumer()
	{
		std::string errstr;

		std::unique_ptr<RdKafka::Conf> conf(
			RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
		if (!conf)
			return "Failed to create config";

		if (conf->set("bootstrap.servers", cfg_.brokers, errstr) != RdKafka::Conf::CONF_OK)
			return errstr;
		if (conf->set("group.id", cfg_.group_id, errstr) != RdKafka::Conf::CONF_OK)
			return errstr;
		if (conf->set("rebalance_cb", &rebalance_cb_, errstr) != RdKafka::Conf::CONF_OK)
			return errstr;
		if (conf->set("enable.auto.commit", "true", errstr) != RdKafka::Conf::CONF_OK)
			return errstr;
		if (conf->set("auto.offset.reset", "earliest", errstr) != RdKafka::Conf::CONF_OK)
			return errstr;

		RdKafka::KafkaConsumer *raw = RdKafka::KafkaConsumer::create(conf.get(), errstr);
		if (!raw)
			return "Failed to create consumer: " + errstr;

		consumer_.reset(raw);

		RdKafka::ErrorCode err = consumer_->subscribe({ cfg_.topic });
		if (err != RdKafka::ERR_NO_ERROR) {
			consumer_.reset();
			return "Subscribe failed: " + RdKafka::err2str(err);
		}

		last_err_msg_.clear();
		last_err_emit_ = std::chrono::steady_clock::time_point{};
		return {};
	}

	void schedule_poll()
	{
		if (!running_)
			return;

		timer_.expires_after(cfg_.poll_interval);
		timer_.async_wait(
			strand_.wrap([self = shared_from_this()](boost::system::error_code ec) {
				if (ec || !self->running_)
					return;
				self->do_poll();
			}));
	}

	void emit_error_throttled(std::string msg)
	{
		auto now = std::chrono::steady_clock::now();

		bool should_emit = false;
		if (msg != last_err_msg_) {
			should_emit = true;
		} else if (last_err_emit_ == std::chrono::steady_clock::time_point{}) {
			should_emit = true;
		} else if (now - last_err_emit_ >= cfg_.error_emit_min_interval) {
			should_emit = true;
		}

		last_err_msg_ = msg;

		if (!should_emit)
			return;

		last_err_emit_ = now;
		sink_(KafkaEvent{ ConnErrorEvent{ std::move(msg) } });
	}

	void do_poll()
	{
		if (!consumer_) {
			emit_error_throttled("consumer not initialised");
			schedule_poll();
			return;
		}

		std::vector<SomeEvent> batch;
		batch.reserve(64);

		for (int i = 0; i < cfg_.max_messages_per_poll; ++i) {
			std::unique_ptr<RdKafka::Message> msg(consumer_->consume(0));
			if (!msg)
				break;

			RdKafka::ErrorCode ec = msg->err();

			if (ec == RdKafka::ERR_NO_ERROR) {
				std::string parse_err;
				if (!parse_actions_append(msg->payload(), msg->len(), batch,
							  parse_err)) {
					emit_error_throttled("parse error: " + parse_err);
				}
				continue;
			}

			if (ec == RdKafka::ERR__TIMED_OUT || ec == RdKafka::ERR__PARTITION_EOF) {
				break;
			}

			emit_error_throttled(RdKafka::err2str(ec) + ": " + msg->errstr());
		}

		consumer_->poll(0);

		if (!batch.empty())
			sink_(KafkaEvent{ ActionsEvent{ std::move(batch) } });

		schedule_poll();
	}
};
