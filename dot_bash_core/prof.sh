#!/bin/bash

# refrence/reminder/convenience funcs for using debuggers/profiling tools
# in apps where its a major pita.
#
# ============================================================================
# thread discovery
# ============================================================================

# list all threads for a process with their names
# usage: threads <pid>
threads() {
    local pid=$1
    if [[ -z "$pid" ]]; then
        echo "usage: threads <pid>" >&2
        return 1
    fi
    ps -T -p "$pid" -o tid,comm,pcpu,state
}

# list threads from /proc (alternative view)
# usage: threads_proc <pid>
threads_proc() {
    local pid=$1
    if [[ -z "$pid" ]]; then
        echo "usage: threads_proc <pid>" >&2
        return 1
    fi
    local t=
    local name=
    for tid in /proc/"$pid"/task/*; do
        t=$(basename "$tid")
        name=$(cat "$tid"/comm 2>/dev/null)
        printf "%6s  %s\n" "$t" "$name"
    done
}

# find thread id by name (partial match)
# usage: tid_by_name <pid> <name_pattern>
tid_by_name() {
    local pid=$1
    local pattern=$2
    if [[ -z "$pid" || -z "$pattern" ]]; then
        echo "usage: tid_by_name <pid> <name_pattern>" >&2
        return 1
    fi
    ps -T -p "$pid" -o tid,comm | grep -i "$pattern" | awk '{print $1}'
}

# ============================================================================
# quick stats
# ============================================================================

# show per-thread cpu usage (requires htop or top)
# usage: thread_cpu <pid>
thread_cpu() {
    local pid=$1
    if [[ -z "$pid" ]]; then
        echo "usage: thread_cpu <pid>" >&2
        return 1
    fi
    top -H -p "$pid" -bn1 | tail -n +8
}

# context switch counts for a process
# usage: ctx_switches <pid>
ctx_switches() {
    local pid=$1
    if [[ -z "$pid" ]]; then
        echo "usage: ctx_switches <pid>" >&2
        return 1
    fi
    grep ctxt /proc/"$pid"/status
}

# file descriptor count
# usage: fd_count <pid>
fd_count() {
    local pid=$1
    if [[ -z "$pid" ]]; then
        echo "usage: fd_count <pid>" >&2
        return 1
    fi
    ls /proc/"$pid"/fd 2>/dev/null | wc -l
}

# ============================================================================
# perf helpers
# ============================================================================

# record cpu profile for entire process
# usage: perf_record <pid> [seconds=30]
perf_record() {
    local pid=$1
    local secs=${2:-30}
    if [[ -z "$pid" ]]; then
        echo "usage: perf_record <pid> [seconds]" >&2
        return 1
    fi
    echo "recording for $secs seconds..." >&2
    perf record -g -p "$pid" -o perf.data -- sleep "$secs"
    echo "done. run 'perf report' to view" >&2
}

# record cpu profile for specific threads
# usage: perf_record_tids <seconds> <tid1> [tid2] [tid3] ...
perf_record_tids() {
    local secs=$1
    shift
    local tids=
    tids=$(IFS=,; echo "$*")
    if [[ -z "$tids" ]]; then
        echo "usage: perf_record_tids <seconds> <tid1> [tid2] ..." >&2
        return 1
    fi
    echo "recording tids $tids for $secs seconds..." >&2
    perf record -g -t "$tids" -o perf.data -- sleep "$secs"
    echo "done. run 'perf report' to view" >&2
}

# quick perf stat overview
# usage: perf_stat <pid> [seconds=10]
perf_stat() {
    local pid=$1
    local secs=${2:-10}
    if [[ -z "$pid" ]]; then
        echo "usage: perf_stat <pid> [seconds]" >&2
        return 1
    fi
    perf stat -e cycles,instructions,cache-misses,context-switches,cpu-migrations \
        -p "$pid" -- sleep "$secs"
}

# perf stat for specific threads
# usage: perf_stat_tids <seconds> <tid1> [tid2] ...
perf_stat_tids() {
    local secs=$1 ; shift
    local tids=
    tids=$(IFS=,; echo "$*")
    if [[ -z "$tids" ]]; then
        echo "usage: perf_stat_tids <seconds> <tid1> [tid2] ..." >&2
        return 1
    fi
    perf stat -e cycles,instructions,cache-misses,context-switches,cpu-migrations \
        -t "$tids" -- sleep "$secs"
}

# ============================================================================
# flamegraph generation
# assumes flamegraph tools are in path (stackcollapse-perf.pl, flamegraph.pl)
# get them from: https://github.com/brendangregg/FlameGraph
# ============================================================================

# generate flamegraph from existing perf.data
# usage: flamegraph [output.svg]
flamegraph() {
    local out="${1:-flame.svg}"
    if [[ ! -f perf.data ]]; then
        echo "no perf.data found. run perf_record first" >&2
        return 1
    fi
    if ! command -v stackcollapse-perf.pl &>/dev/null; then
        echo "flamegraph tools not found in path" >&2
        echo "get them: git clone https://github.com/brendangregg/FlameGraph" >&2
        return 1
    fi
    perf script | stackcollapse-perf.pl | flamegraph.pl > "$out"
    echo "wrote $out" >&2
}

# record and generate flamegraph in one step
# usage: quick_flame <pid> [seconds=30] [output.svg]
quick_flame() {
    local pid=$1
    local secs=${2:-30}
    local out=${3:-flame.svg}
    if [[ -z "$pid" ]]; then
        echo "usage: quick_flame <pid> [seconds] [output.svg]" >&2
        return 1
    fi
    perf_record "$pid" "$secs" && flamegraph "$out"
}

# flamegraph filtered to specific thread from existing perf.data
# usage: flamegraph_tid <tid> [output.svg]
flamegraph_tid() {
    local tid=$1
    local out=${2:-flame_$tid.svg}
    if [[ -z "$tid" ]]; then
        echo "usage: flamegraph_tid <tid> [output.svg]" >&2
        return 1
    fi
    if [[ ! -f perf.data ]]; then
        echo "no perf.data found. run perf_record first" >&2
        return 1
    fi
    # filter perf script output to just this tid
    # format: process tid [time] event: ...
    perf script | awk -v tid="$tid" '
        /^[^ ]/ {
            # new record line, check if it matches our tid
            match($0, /[0-9]+\/([0-9]+)/, arr)
            if (arr[1] == tid) { printing=1 } else { printing=0 }
        }
        printing { print }
    ' | stackcollapse-perf.pl | flamegraph.pl > "$out"
    echo "wrote $out" >&2
}

# ============================================================================
# strace helpers
# ============================================================================

# syscall summary for a process
# usage: syscall_summary <pid> [seconds=10]
syscall_summary() {
    local pid=$1
    local secs=${2:-10}
    if [[ -z "$pid" ]]; then
        echo "usage: syscall_summary <pid> [seconds]" >&2
        return 1
    fi
    timeout "$secs" strace -c -p "$pid" 2>&1
}

# trace io syscalls
# usage: trace_io <pid> [seconds=10]
trace_io() {
    local pid=$1
    local secs=${2:-10}
    if [[ -z "$pid" ]]; then
        echo "usage: trace_io <pid> [seconds]" >&2
        return 1
    fi
    timeout "$secs" strace -f -p "$pid" \
        -e trace=read,write,sendto,recvfrom,epoll_wait,epoll_ctl \
        2>&1 | head -500
}

# trace network syscalls only
# usage: trace_net <pid> [seconds=10]
trace_net() {
    local pid=$1
    local secs=${2:-10}
    if [[ -z "$pid" ]]; then
        echo "usage: trace_net <pid> [seconds]" >&2
        return 1
    fi
    timeout "$secs" strace -f -p "$pid" \
        -e trace=socket,connect,accept,accept4,bind,listen,sendto,recvfrom,sendmsg,recvmsg \
        2>&1 | head -500
}

# ============================================================================
# bpftrace helpers (require root)
# ============================================================================

# histogram of epoll_wait durations for a process
# usage: bpf_epoll_hist <pid> [seconds=10]
bpf_epoll_hist() {
    local pid=$1
    local secs=${2:-10}
    if [[ -z "$pid" ]]; then
        echo "usage: bpf_epoll_hist <pid> [seconds]" >&2
        return 1
    fi
    sudo timeout "$secs" bpftrace -e "
        tracepoint:syscalls:sys_enter_epoll_wait /pid == $pid/ {
            @start[tid] = nsecs;
        }
        tracepoint:syscalls:sys_exit_epoll_wait /pid == $pid && @start[tid]/ {
            @epoll_us = hist((nsecs - @start[tid]) / 1000);
            delete(@start[tid]);
        }
        END { clear(@start); }
    "
}

# histogram of epoll_wait durations for specific thread
# usage: bpf_epoll_hist_tid <tid> [seconds=10]
bpf_epoll_hist_tid() {
    local tid=$1
    local secs=${2:-10}
    if [[ -z "$tid" ]]; then
        echo "usage: bpf_epoll_hist_tid <tid> [seconds]" >&2
        return 1
    fi
    sudo timeout "$secs" bpftrace -e "
        tracepoint:syscalls:sys_enter_epoll_wait /tid == $tid/ {
            @start[tid] = nsecs;
        }
        tracepoint:syscalls:sys_exit_epoll_wait /tid == $tid && @start[tid]/ {
            @epoll_us = hist((nsecs - @start[tid]) / 1000);
            delete(@start[tid]);
        }
        END { clear(@start); }
    "
}

# count syscalls by thread
# usage: bpf_syscall_by_thread <pid> [seconds=10]
bpf_syscall_by_thread() {
    local pid=$1
    local secs=${2:-10}
    if [[ -z "$pid" ]]; then
        echo "usage: bpf_syscall_by_thread <pid> [seconds]" >&2
        return 1
    fi
    sudo timeout "$secs" bpftrace -e "
        tracepoint:raw_syscalls:sys_enter /pid == $pid/ {
            @[tid, comm] = count();
        }
    "
}

# ============================================================================
# gdb helpers
# ============================================================================

# attach to process and dump all thread backtraces
# usage: dump_all_stacks <pid>
dump_all_stacks() {
    local pid=$1
    if [[ -z "$pid" ]]; then
        echo "usage: dump_all_stacks <pid>" >&2
        return 1
    fi
    gdb -batch -p "$pid" -ex "thread apply all bt" 2>/dev/null
}

# attach and dump backtrace for specific thread
# usage: dump_thread_stack <pid> <tid>
dump_thread_stack() {
    local pid=$1
    local tid=$2
    if [[ -z "$pid" || -z "$tid" ]]; then
        echo "usage: dump_thread_stack <pid> <tid>" >&2
        return 1
    fi
    # find gdb thread number from tid - this is approximate
    gdb -batch -p "$pid" \
        -ex "info threads" \
        -ex "thread apply all bt" 2>/dev/null | \
        grep -A50 "LWP $tid"
}

# ============================================================================
# combined workflows
# ============================================================================

# quick overview of a running process
# usage: proc_overview <pid>
proc_overview() {
    local pid=$1
    if [[ -z "$pid" ]]; then
        echo "usage: proc_overview <pid>" >&2
        return 1
    fi

    {
        echo "=== process info ==="
        ps -p "$pid" -o pid,comm,pcpu,pmem,rss,vsz,state

        echo ""
        echo "=== threads (top 10 by cpu) ==="
        ps -T -p "$pid" -o tid,comm,pcpu,state --sort=-pcpu | head -12

        echo ""
        echo "=== context switches ==="
        ctx_switches "$pid"

        echo ""
        echo "=== file descriptors ==="
        echo "count: $(fd_count "$pid")"

        echo ""
        echo "=== memory maps summary ==="
        wc -l < /proc/"$pid"/maps
        echo "mapping regions"
    } >&2
}

# identify likely bottleneck thread
# usage: find_hot_thread <pid>
find_hot_thread() {
    local pid=$1
    if [[ -z "$pid" ]]; then
        echo "usage: find_hot_thread <pid>" >&2
        return 1
    fi
    local hot_tid=
    {
        echo "highest cpu threads:"
        ps -T -p "$pid" -o tid,comm,pcpu --sort=-pcpu | head -5
        echo ""
        echo "to profile the top one:"
        hot_tid=$(ps -T -p "$pid" -o tid --sort=-pcpu --no-headers | head -1 | tr -d ' ')
        echo "  perf_record_tids 30 $hot_tid"
    } >&2
}

# profile controller thread specifically (assumes thread named 'controller')
# usage: profile_controller <pid> [seconds=30]
profile_controller() {
    local pid=$1
    local secs=${2:-30}
    local tid=
    if [[ -z "$pid" ]]; then
        echo "usage: profile_controller <pid> [seconds]" >&2
        return 1
    fi
    tid=$(tid_by_name "$pid" "controller")
    if [[ -z "$tid" ]]; then
        {
            echo "no thread named 'controller' found"
            echo "available threads:"
            threads "$pid"
        } >&2
        return 1
    fi
    echo "found controller thread: $tid" >&2
    perf_record_tids "$secs" "$tid"
}

# ============================================================================
# utility
# ============================================================================

# watch thread cpu usage in real-time
# usage: watch_threads <pid>
watch_threads() {
    local pid=$1
    if [[ -z "$pid" ]]; then
        echo "usage: watch_threads <pid>" >&2
        return 1
    fi
    watch -n1 "ps -T -p $pid -o tid,comm,pcpu,state --sort=-pcpu | head -20"
}

# list these helper functions
perf_help() {
    cat >&2 <<'EOF'
thread discovery:
  threads <pid>              - list all threads
  threads_proc <pid>         - list threads from /proc
  tid_by_name <pid> <name>   - find tid by name pattern

quick stats:
  thread_cpu <pid>           - per-thread cpu usage
  ctx_switches <pid>         - context switch counts
  fd_count <pid>             - file descriptor count

perf:
  perf_record <pid> [secs]   - record cpu profile
  perf_record_tids <secs> <tid...> - record specific threads
  perf_stat <pid> [secs]     - quick perf counters
  perf_stat_tids <secs> <tid...>   - perf stat specific threads

flamegraphs:
  flamegraph [out.svg]       - generate from perf.data
  quick_flame <pid> [secs] [out.svg] - record and generate
  flamegraph_tid <tid> [out.svg]    - filter to one thread

strace:
  syscall_summary <pid> [secs] - syscall counts
  trace_io <pid> [secs]        - trace io syscalls
  trace_net <pid> [secs]       - trace network syscalls

bpftrace (needs root):
  bpf_epoll_hist <pid> [secs]      - epoll_wait histogram
  bpf_epoll_hist_tid <tid> [secs]  - epoll_wait for one thread
  bpf_syscall_by_thread <pid> [secs] - syscalls per thread

gdb:
  dump_all_stacks <pid>           - all thread backtraces
  dump_thread_stack <pid> <tid>   - one thread backtrace

workflows:
  proc_overview <pid>        - quick process summary
  find_hot_thread <pid>      - identify cpu-heavy threads
  profile_controller <pid> [secs] - profile 'controller' thread
  watch_threads <pid>        - live thread cpu monitor
EOF
}

echo "profiling helpers loaded. run 'perf_help' for available functions." >&2
