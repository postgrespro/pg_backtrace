#include <execinfo.h>
#include <unistd.h>
#include <signal.h>

#include "postgres.h"
#include "miscadmin.h"
#include "utils/guc.h"
#include "executor/executor.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

#define MAX_BACK_TRACE_DEPTH    100
#define SKIP_FRAMES             3

/* pg module functions */
void _PG_init(void);
void _PG_fini(void);

PG_FUNCTION_INFO_V1(pg_backtrace_init);
PG_FUNCTION_INFO_V1(pg_backtrace_sigsegv);

static int backtrace_level = ERROR;
static ErrorContextCallback backtrace_callback;
static ExecutorRun_hook_type prev_executor_run_hook;
static bool inside_signal_handler;
static pqsigfunc signal_handlers[_NSIG];

static void
backtrace_dump_stack(void)
{
    void* back_trace[MAX_BACK_TRACE_DEPTH];
    int depth = backtrace(back_trace, MAX_BACK_TRACE_DEPTH);
    char** stack = backtrace_symbols(back_trace+SKIP_FRAMES, depth-SKIP_FRAMES);
    int i;
    for (i = 0; i < depth-SKIP_FRAMES; i++) {
		errcontext_msg("\t%s", stack[i]);
    }
    free(stack);
}

static void
backtrace_callback_function(void* arg)
{
	if (inside_signal_handler || geterrcode() >= backtrace_level)
	{
		backtrace_dump_stack();
	}
}

static void
backtrace_executor_run_hook(QueryDesc *queryDesc,
							ScanDirection direction,
							uint64 count,
							bool execute_once)
{
	backtrace_callback.callback = backtrace_callback_function;
	backtrace_callback.previous = error_context_stack;
	error_context_stack = &backtrace_callback;
	if (prev_executor_run_hook)
		(*prev_executor_run_hook)(queryDesc, direction, count, execute_once);
	else
		standard_ExecutorRun(queryDesc, direction, count, execute_once);
}



static void
backtrace_handler(SIGNAL_ARGS)
{
	inside_signal_handler = true;
	elog(LOG, "Caught signal %d", postgres_signal_arg);
	inside_signal_handler = false;
	if (postgres_signal_arg != SIGINT)
		signal_handlers[postgres_signal_arg](postgres_signal_arg);
}

static const struct config_enum_entry backtrace_level_options[] =
{
	{"debug5", DEBUG5, false},
	{"debug4", DEBUG4, false},
	{"debug3", DEBUG3, false},
	{"debug2", DEBUG2, false},
	{"debug1", DEBUG1, false},
	{"debug", DEBUG2, true},
	{"log", LOG, false},
	{"info", INFO, true},
	{"notice", NOTICE, false},
	{"warning", WARNING, false},
	{"error", ERROR, false},
	{"fatal", FATAL, true},
	{"panic", PANIC, true},
	{NULL, 0, false}
};

void _PG_init(void)
{
	signal_handlers[SIGSEGV] = pqsignal(SIGSEGV, backtrace_handler);
	signal_handlers[SIGBUS] = pqsignal(SIGBUS, backtrace_handler);
	signal_handlers[SIGINT] = pqsignal(SIGINT, backtrace_handler);
	prev_executor_run_hook = ExecutorRun_hook;
	ExecutorRun_hook = backtrace_executor_run_hook;

    DefineCustomEnumVariable("pg_backtrace.level",
							 "Set error level for dumping backtrace",
							 NULL,
							 &backtrace_level,
							 ERROR,
							 backtrace_level_options,
							 PGC_USERSET, 0, NULL, NULL, NULL);
}

void _PG_fini(void)
{
	ExecutorRun_hook = prev_executor_run_hook;
	pqsignal(SIGSEGV, signal_handlers[SIGSEGV]);
	pqsignal(SIGBUS,  signal_handlers[SIGBUS]);
	pqsignal(SIGINT,  signal_handlers[SIGINT]);
}

Datum pg_backtrace_init(PG_FUNCTION_ARGS);
Datum pg_backtrace_sigsegv(PG_FUNCTION_ARGS);


Datum pg_backtrace_init(PG_FUNCTION_ARGS)
{
	PG_RETURN_VOID();
}

Datum pg_backtrace_sigsegv(PG_FUNCTION_ARGS)
{
	*(int*)0 = 0;
	PG_RETURN_VOID();
}
