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

#include <boost/asio.hpp>
#include <librdkafka/rdkafka.h>

{
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <variant>

	struct KafkaDelivered {
		std::uint64_t token;
		std::int32_t partition;
		std::int64_t offset;
		std::int64_t timestamp_ms;
	};

	struct KafkaFailed {
		std::uint64_t token;
		int err;
		std::string msg;
		bool retriable;
	};

	using KafkaProduceEvent = std::variant<KafkaDelivered, KafkaFailed>;
	using KafkaProduceSink = std::function<void(KafkaProduceEvent &&)>;

	class KafkaProducer : public std::enable_shared_from_this<KafkaProducer> {
	    public:
		struct Config {
			std::string brokers;
			std::string topic;
			std::int32_t partition;
			std::string client_id;
			std::chrono::milliseconds poll_interval{ 50 };
			std::size_t max_pending_msgs{ 16384 };
			std::size_t max_pending_bytes{ 16u * 1024u * 1024u };
			bool enable_idempotence{ false };
		};

		struct Msg {
			std::uint64_t token;
			std::string payload;
		};

		KafkaProducer(boost::asio::io_service &ios, Config cfg, KafkaProduceSink sink)
			: m_strand(ios)
			, m_timerg(ios)
			, m_cfg(std::move(cfg))
			, m_sink(std::move(sink))
		{
		}

		~KafkaProducer()
		{
			stop();
		}

		bool start(std::string &err)
		{
			bool ok;

			err.clear();
			ok = init_producer(err);
			if (!ok)
				return false;

			running_ = true;
			schedule_tick();
			return true;
		}

		void stop()
		{
			auto wk = weak_from_this();
			m_strand.post([wk]() {
				if (auto self = wk.lock())
					self->do_stop();
			});
		}

		void send(Msg &&m)
		{
			auto wk = weak_from_this();
			m_strand.post([wk, m = std::move(m)]() mutable {
				if (auto self = wk.lock())
					self->send_on_strand(std::move(m));
			});
		}

	    private:
		boost::asio::io_service::strand m_strand;
		boost::asio::steady_timer m_timerg;
		Config m_cfg;
		KafkaProduceSink m_sink;

		bool running_ = false;

		rd_kafka_t *rk_ = nullptr;

		std::deque<Msg> pending_;
		std::size_t pending_bytes_ = 0;

		static_assert(sizeof(void *) >= 8,
			      "requires 64-bit pointers for token opaque packing");

		static void *opaque_from_token(std::uint64_t token)
		{
			return reinterpret_cast<void *>(static_cast<std::uintptr_t>(token));
		}

		static std::uint64_t token_from_opaque(void *p)
		{
			return static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(p));
		}

		static void dr_msg_cb(rd_kafka_t *rk, const rd_kafka_message_t *rkmsg, void *opaque)
		{
			KafkaProducer *self;
			std::uint64_t token;
			std::int64_t ts;
			rd_kafka_timestamp_type_t tst;
			int err;
			bool retriable;
			char buf[256];

			(void)rk;

			self = static_cast<KafkaProducer *>(opaque);
			if (!self)
				return;

			token = token_from_opaque(rd_kafka_message_opaque(rkmsg));
			ts = rd_kafka_message_timestamp(rkmsg, &tst);
			if (ts < 0)
				ts = 0;

			err = rkmsg->err;
			if (err == 0) {
				self->m_sink(KafkaProduceEvent{ KafkaDelivered{
					token, rkmsg->partition, rkmsg->offset, ts } });
				return;
			}

			retriable = false;
			{
				rd_kafka_error_t *e = rd_kafka_message_error(rkmsg);
				if (e)
					retriable = rd_kafka_error_is_retriable(e) ? true : false;
			}

			std::snprintf(buf, sizeof(buf), "%s",
				      rd_kafka_err2str(static_cast<rd_kafka_resp_err_t>(err)));

			self->m_sink(KafkaProduceEvent{
				KafkaFailed{ token, err, std::string(buf), retriable } });
		}

		bool init_producer(std::string &err)
		{
			rd_kafka_conf_t *conf;
			char errbuf[512];
			int rc;

			if (rk_) {
				err.clear();
				return true;
			}

			conf = rd_kafka_conf_new();

			rd_kafka_conf_set_dr_msg_cb(conf, &KafkaProducer::dr_msg_cb);
			rd_kafka_conf_set_opaque(conf, this);

			rc = rd_kafka_conf_set(conf, "bootstrap.servers", m_cfg.brokers.c_str(),
					       errbuf, sizeof(errbuf));
			if (rc != RD_KAFKA_CONF_OK) {
				err = errbuf;
				rd_kafka_conf_destroy(conf);
				return false;
			}

			if (!m_cfg.client_id.empty()) {
				rc = rd_kafka_conf_set(conf, "client.id", m_cfg.client_id.c_str(),
						       errbuf, sizeof(errbuf));
				if (rc != RD_KAFKA_CONF_OK) {
					err = errbuf;
					rd_kafka_conf_destroy(conf);
					return false;
				}
			}

			if (m_cfg.enable_idempotence) {
				rc = rd_kafka_conf_set(conf, "enable.idempotence", "true", errbuf,
						       sizeof(errbuf));
				if (rc != RD_KAFKA_CONF_OK) {
					err = errbuf;
					rd_kafka_conf_destroy(conf);
					return false;
				}
			}

			rk_ = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errbuf, sizeof(errbuf));
			if (!rk_) {
				err = errbuf;
				rd_kafka_conf_destroy(conf);
				return false;
			}

			err.clear();
			return true;
		}

		void do_stop()
		{
			std::size_t i;
			Msg m;

			if (!running_ && !rk_)
				return;

			running_ = false;

			{
				boost::system::error_code ignored;
				m_timerg.cancel(ignored);
			}

			for (i = 0; i < pending_.size(); ++i) {
				m = std::move(pending_[i]);
				m_sink(KafkaProduceEvent{ KafkaFailed{
					m.token, static_cast<int>(RD_KAFKA_RESP_ERR__DESTROY),
					std::string("producer stopped"), false } });
			}
			pending_.clear();
			pending_bytes_ = 0;

			if (rk_) {
				rd_kafka_destroy(rk_);
				rk_ = nullptr;
			}
		}

		void schedule_tick()
		{
			if (!running_)
				return;

			m_timerg.expires_from_now(m_cfg.poll_interval);
			m_timerg.async_wait(m_strand.wrap(
				[self = shared_from_this()](const boost::system::error_code &ec) {
					if (ec || !self->running_)
						return;
					self->on_tick();
				}));
		}

		void on_tick()
		{
			std::size_t drained;
			drained = 0;

			if (rk_)
				rd_kafka_poll(rk_, 0);

			while (!pending_.empty()) {
				if (!try_produce_now(pending_.front())) {
					break;
				}
				pending_bytes_ -= pending_.front().payload.size();
				pending_.pop_front();
				drained++;
				if (drained >= 1024)
					break;
			}

			if (rk_)
				rd_kafka_poll(rk_, 0);

			schedule_tick();
		}

		void send_on_strand(Msg &&m)
		{
			bool ok;
			std::size_t sz;

			if (!running_ || !rk_) {
				m_sink(KafkaProduceEvent{ KafkaFailed{
					m.token, static_cast<int>(RD_KAFKA_RESP_ERR__STATE),
					std::string("producer not running"), false } });
				return;
			}

			ok = try_produce_now(m);
			if (ok)
				return;

			sz = m.payload.size();

			if (pending_.size() >= m_cfg.max_pending_msgs ||
			    (pending_bytes_ + sz) > m_cfg.max_pending_bytes) {
				m_sink(KafkaProduceEvent{ KafkaFailed{
					m.token, static_cast<int>(RD_KAFKA_RESP_ERR__QUEUE_FULL),
					std::string("local pending queue overflow"), true } });
				return;
			}

			pending_bytes_ += sz;
			pending_.push_back(std::move(m));
		}

		bool try_produce_now(Msg &m)
		{
			int r;
			rd_kafka_resp_err_t err;
			bool retriable;

			r = rd_kafka_producev(rk_, RD_KAFKA_V_TOPIC(m_cfg.topic.c_str()),
					      RD_KAFKA_V_PARTITION(m_cfg.partition),
					      RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
					      RD_KAFKA_V_VALUE(const_cast<char *>(m.payload.data()),
							       m.payload.size()),
					      RD_KAFKA_V_OPAQUE(opaque_from_token(m.token)),
					      RD_KAFKA_V_END);

			if (r == 0)
				return true;

			err = rd_kafka_last_error();

			if (err == RD_KAFKA_RESP_ERR__QUEUE_FULL)
				return false;

			retriable = false;
			{
				rd_kafka_error_t *e = rd_kafka_error_new(err);
				if (e) {
					retriable = rd_kafka_error_is_retriable(e) ? true : false;
					rd_kafka_error_destroy(e);
				}
			}

			m_sink(KafkaProduceEvent{ KafkaFailed{ m.token, static_cast<int>(err),
							       std::string(rd_kafka_err2str(err)),
							       retriable } });

			return true;
		}
	};
}
