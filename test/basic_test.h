#ifndef NANODBC_TEST_BASIC_TEST_H
#define NANODBC_TEST_BASIC_TEST_H

#include "nanodbc.h"
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#ifdef NANODBC_USE_UNICODE
    #define NANODBC_TEXT(s) L ## s
#else
    #define NANODBC_TEXT(s) s
#endif

struct basic_test
{
    typedef boost::mpl::list<
        nanodbc::string_type::value_type
        , short
        , unsigned short
        , int32_t
        , uint32_t
        , int64_t
        , uint64_t
        , float
        , double
        > integral_test_types;

    basic_test(const nanodbc::string_type& connection_string)
    : connection_string_(connection_string)
    {

    }

    nanodbc::connection connect()
    {
        return nanodbc::connection(connection_string_);
    }

    void simple_test()
    {
        nanodbc::connection connection = connect();
        BOOST_CHECK(connection.connected());
        BOOST_CHECK(connection.native_dbc_handle());
        BOOST_CHECK(connection.native_env_handle());
        BOOST_CHECK_EQUAL(connection.transactions(), 0);

        execute(connection, NANODBC_TEXT("drop table if exists simple_test;"));
        execute(connection, NANODBC_TEXT("create table simple_test (a int, b varchar(10));"));
        execute(connection, NANODBC_TEXT("insert into simple_test values (1, 'one');"));
        execute(connection, NANODBC_TEXT("insert into simple_test values (2, 'two');"));
        execute(connection, NANODBC_TEXT("insert into simple_test values (3, 'tri');"));
        execute(connection, NANODBC_TEXT("insert into simple_test (b) values ('z');"));

        {
            nanodbc::result results = execute(connection, NANODBC_TEXT("select a, b from simple_test order by a;"));

            BOOST_CHECK((bool)results);
            BOOST_CHECK_EQUAL(results.rows(), 0);
            BOOST_CHECK_EQUAL(results.columns(), 2);
            BOOST_CHECK_EQUAL(results.affected_rows(), 0);
            BOOST_CHECK_EQUAL(results.rowset_size(), 1);
            BOOST_CHECK(results.column_name(0) == NANODBC_TEXT("a"));
            BOOST_CHECK(results.column_name(1) == NANODBC_TEXT("b"));

            BOOST_CHECK(results.next());
            BOOST_CHECK_EQUAL(results.rows(), 1);
            BOOST_CHECK(results.is_null(0));
            BOOST_CHECK(results.is_null(NANODBC_TEXT("a")));
            BOOST_CHECK_EQUAL(results.get<int>(0, -1), -1);
            BOOST_CHECK_EQUAL(results.get<int>(NANODBC_TEXT("a"), -1), -1);
            BOOST_CHECK(results.get<nanodbc::string_type>(0, NANODBC_TEXT("null")) == NANODBC_TEXT("null"));
            BOOST_CHECK(results.get<nanodbc::string_type>(NANODBC_TEXT("a"), NANODBC_TEXT("null")) == NANODBC_TEXT("null"));
            BOOST_CHECK(results.get<nanodbc::string_type>(1) == NANODBC_TEXT("z"));
            BOOST_CHECK(results.get<nanodbc::string_type>(NANODBC_TEXT("b")) == NANODBC_TEXT("z"));

            int ref_int;
            results.get_ref(0, -1, ref_int);
            BOOST_CHECK_EQUAL(ref_int, -1);
            results.get_ref(NANODBC_TEXT("a"), -2, ref_int);
            BOOST_CHECK_EQUAL(ref_int, -2);

            nanodbc::string_type ref_str;
            results.get_ref<nanodbc::string_type>(0, NANODBC_TEXT("null"), ref_str);
            BOOST_CHECK_EQUAL(ref_str, NANODBC_TEXT("null"));
            results.get_ref<nanodbc::string_type>(NANODBC_TEXT("a"), NANODBC_TEXT("null2"), ref_str);
            BOOST_CHECK_EQUAL(ref_str, NANODBC_TEXT("null2"));

            BOOST_CHECK(results.next());
            BOOST_CHECK_EQUAL(results.get<int>(0), 1);
            BOOST_CHECK_EQUAL(results.get<int>(NANODBC_TEXT("a")), 1);
            BOOST_CHECK(results.get<nanodbc::string_type>(1) == NANODBC_TEXT("one"));
            BOOST_CHECK(results.get<nanodbc::string_type>(NANODBC_TEXT("b")) == NANODBC_TEXT("one"));

            nanodbc::result results_copy = results;

            BOOST_CHECK(results_copy.next());
            BOOST_CHECK_EQUAL(results_copy.get<int>(0, -1), 2);
            BOOST_CHECK_EQUAL(results_copy.get<int>(NANODBC_TEXT("a"), -1), 2);
            BOOST_CHECK(results_copy.get<nanodbc::string_type>(1) == NANODBC_TEXT("two"));
            BOOST_CHECK(results_copy.get<nanodbc::string_type>(NANODBC_TEXT("b")) == NANODBC_TEXT("two"));

            BOOST_CHECK(results.position());

            nanodbc::result().swap(results_copy);

            BOOST_CHECK(results.next());
            BOOST_CHECK(results.get<nanodbc::string_type>(0) == NANODBC_TEXT("3"));
            BOOST_CHECK(results.get<nanodbc::string_type>(NANODBC_TEXT("a")) == NANODBC_TEXT("3"));
            BOOST_CHECK(results.get<nanodbc::string_type>(1) == NANODBC_TEXT("tri"));
            BOOST_CHECK(results.get<nanodbc::string_type>(NANODBC_TEXT("b")) == NANODBC_TEXT("tri"));

            BOOST_CHECK(!results.next());
            BOOST_CHECK(results.end());
        }

        nanodbc::connection connection_copy(connection);

        connection.disconnect();
        BOOST_CHECK(!connection.connected());
        BOOST_CHECK(!connection_copy.connected());
    }

    void string_test()
    {
        nanodbc::connection connection = connect();
        BOOST_CHECK(connection.connected());
        BOOST_CHECK(connection.native_dbc_handle());
        BOOST_CHECK(connection.native_env_handle());
        BOOST_CHECK_EQUAL(connection.transactions(), 0);

        const nanodbc::string_type name = NANODBC_TEXT("Fred");

        execute(connection, NANODBC_TEXT("drop table if exists string_test;"));
        execute(connection, NANODBC_TEXT("create table string_test (s varchar(10));"));

        nanodbc::statement query(connection);
        prepare(query, NANODBC_TEXT("insert into string_test(s) values(?)"));
        query.bind(0, name.c_str());
        nanodbc::execute(query);

        nanodbc::result results = execute(connection, NANODBC_TEXT("select s from string_test;"));
        BOOST_CHECK(results.next());
        BOOST_CHECK(results.get<nanodbc::string_type>(0) == NANODBC_TEXT("Fred"));

        nanodbc::string_type ref;
        results.get_ref(0, ref);
        BOOST_CHECK(ref == name);
    }

    static void check_rows_equal(nanodbc::result results, std::size_t rows)
    {
        BOOST_CHECK(results.next());
        BOOST_CHECK_EQUAL(results.get<int>(0), rows);
    }

    void transaction_test()
    {
        nanodbc::connection connection = connect();
        BOOST_CHECK(connection.connected());

        execute(connection, NANODBC_TEXT("drop table if exists transaction_test;"));
        execute(connection, NANODBC_TEXT("create table transaction_test (i int);"));

        nanodbc::statement statement(connection);
        prepare(statement, NANODBC_TEXT("insert into transaction_test (i) values (?);"));

        static const int elements = 10;
        int data[elements] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        statement.bind(0, data, elements);
        execute(statement, elements);

        static const nanodbc::string_type::value_type* query = NANODBC_TEXT("select count(1) from transaction_test;");

        check_rows_equal(execute(connection, query), 10);

        {
            nanodbc::transaction transaction(connection);
            execute(connection, NANODBC_TEXT("delete from transaction_test;"));
            check_rows_equal(execute(connection, query), 0);
            // ~transaction() calls rollback()
        }

        check_rows_equal(execute(connection, query), 10);

        {
            nanodbc::transaction transaction(connection);
            execute(connection, NANODBC_TEXT("delete from transaction_test;"));
            check_rows_equal(execute(connection, query), 0);
            transaction.rollback();
        }

        check_rows_equal(execute(connection, query), 10);

        {
            nanodbc::transaction transaction(connection);
            execute(connection, NANODBC_TEXT("delete from transaction_test;"));
            check_rows_equal(execute(connection, query), 0);
            transaction.commit();
        }

        check_rows_equal(execute(connection, query), 0);
    }

    void exception_test()
    {
        nanodbc::connection connection = connect();
        nanodbc::result results;

        BOOST_CHECK_THROW(execute(connection, NANODBC_TEXT("THIS IS NOT VALID SQL!")), nanodbc::database_error);

        execute(connection,
            NANODBC_TEXT("drop table if exists exception_test;")
            NANODBC_TEXT("create table exception_test (i int);")
            NANODBC_TEXT("insert into exception_test values (-10);")
            NANODBC_TEXT("insert into exception_test values (null);")
        );

        results = execute(connection, NANODBC_TEXT("select * from exception_test where i = -10;"));

        BOOST_CHECK(results.next());
        BOOST_CHECK_THROW(results.get<nanodbc::date>(0), nanodbc::type_incompatible_error);
        BOOST_CHECK_THROW(results.get<nanodbc::timestamp>(0), nanodbc::type_incompatible_error);

        results = execute(connection, NANODBC_TEXT("select * from exception_test where i is null;"));

        BOOST_CHECK(results.next());
        BOOST_CHECK_THROW(results.get<int>(0), nanodbc::null_access_error);
        BOOST_CHECK_THROW(results.get<int>(42), nanodbc::index_range_error);

        nanodbc::statement statement(connection);
        BOOST_CHECK(statement.open() && statement.connected());
        statement.close();
        BOOST_CHECK_THROW(statement.prepare(NANODBC_TEXT("select * from exception_test;")), nanodbc::programming_error);
    }

    void execute_multiple()
    {
        nanodbc::connection connection = connect();
        nanodbc::statement statement(connection);
        nanodbc::prepare(statement, NANODBC_TEXT("select 42;"));

        nanodbc::result results = statement.execute();
        results.next();

        results = statement.execute();
        results.next();
        BOOST_CHECK_EQUAL(results.get<int>(0), 42);

        results = statement.execute();
        results.next();
        BOOST_CHECK_EQUAL(results.get<int>(0), 42);
    }

    void execute_multiple_transaction()
    {
        nanodbc::connection connection = connect();
        nanodbc::statement statement;
        nanodbc::result results;

        statement.prepare(connection, NANODBC_TEXT("select 42;"));

        {
            nanodbc::transaction transaction(connection);
            results = statement.execute();   
            results.next();
            BOOST_CHECK_EQUAL(results.get<int>(0), 42);
        }

        results = statement.execute();
        results.next();
        BOOST_CHECK_EQUAL(results.get<int>(0), 42);
    }

    void null_test()
    {
        nanodbc::connection connection = connect();
        BOOST_CHECK(connection.connected());

        execute(connection, NANODBC_TEXT("drop table if exists null_test;"));
        execute(connection, NANODBC_TEXT("create table null_test (a int, b varchar(10));"));

        nanodbc::statement statement(connection);

        prepare(statement, NANODBC_TEXT("insert into null_test (a, b) values (?, ?);"));
        statement.bind_null(0);
        statement.bind_null(1);
        execute(statement);

        prepare(statement, NANODBC_TEXT("insert into null_test (a, b) values (?, ?);"));
        statement.bind_null(0, 2);
        statement.bind_null(1, 2);
        execute(statement, 2);

        nanodbc::result results = execute(connection, NANODBC_TEXT("select a, b from null_test order by a;"));

        BOOST_CHECK(results.next());
        BOOST_CHECK(results.is_null(0));
        BOOST_CHECK(results.is_null(1));

        BOOST_CHECK(results.next());
        BOOST_CHECK(results.is_null(0));
        BOOST_CHECK(results.is_null(1));

        BOOST_CHECK(results.next());
        BOOST_CHECK(results.is_null(0));
        BOOST_CHECK(results.is_null(1));

        BOOST_CHECK(!results.next());
    }

    template<class T>
    void integral_test_template()
    {
        nanodbc::connection connection = connect();

        execute(connection, NANODBC_TEXT("drop table if exists integral_test;"));
        execute(connection, NANODBC_TEXT("create table integral_test (i int, f float, d double precision);"));

        nanodbc::statement statement(connection);
        prepare(statement, NANODBC_TEXT("insert into integral_test (i, f, d) values (?, ?, ?);"));

        srand(0);
        const int32_t i = rand() % 5000;
        const float f = rand() / (rand() + 1.0);
        const float d = - rand() / (rand() + 1.0);

        short p = 0;
        statement.bind(p++, &i);
        statement.bind(p++, &f);
        statement.bind(p++, &d);

        BOOST_CHECK(statement.connected());
        execute(statement);

        nanodbc::result results = execute(connection, NANODBC_TEXT("select * from integral_test;"));
        BOOST_CHECK(results.next());

        T ref;
        p = 0;
        results.get_ref(p, ref);
        BOOST_CHECK_EQUAL(ref, static_cast<T>(i));
        BOOST_CHECK_EQUAL(results.get<T>(p++), static_cast<T>(i));
        results.get_ref(p, ref);
        BOOST_CHECK_CLOSE(static_cast<float>(ref), static_cast<T>(f), 1e-6);
        BOOST_CHECK_CLOSE(static_cast<float>(results.get<T>(p++)), static_cast<T>(f), 1e-6);
        results.get_ref(p, ref);
        BOOST_CHECK_CLOSE(static_cast<double>(ref), static_cast<T>(d), 1e-6);
        BOOST_CHECK_CLOSE(static_cast<double>(results.get<T>(p++)), static_cast<T>(d), 1e-6);        
    }

    const nanodbc::string_type connection_string_;
};

#endif // NANODBC_TEST_BASIC_TEST_H
