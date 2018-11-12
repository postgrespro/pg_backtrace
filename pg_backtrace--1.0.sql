/* contrib/pg_backtrace/pg_backtrace--1.1.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pg_backtrace" to load this file. \quit

create function pg_backtrace_init() returns void as 'MODULE_PATHNAME' language C strict;
create function pg_backtrace_sigsegv() returns void as 'MODULE_PATHNAME' language C strict;
