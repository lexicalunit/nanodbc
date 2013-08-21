//! \file nanodbc.h The entirety of nanodbc can be found within this file and nanodbc.cpp.

//! \mainpage
//! 
//! \section toc Table of Contents
//! \li \ref license "License"
//! \li \ref credits "Credits"
//! \li \ref examples "Example Usage"
//! \li \ref nanodbc "Namespace Reference"
//! \li <a href="http://lexicalunit.github.com/nanodbc/">Project Homepage</a>
//! 
//! \section license License
//! Copyright (C) 2013 lexicalunit <amy@lexicalunit.com>
//! 
//! The MIT License
//! 
//! Permission is hereby granted, free of charge, to any person obtaining a copy
//! of this software and associated documentation files (the "Software"), to deal
//! in the Software without restriction, including without limitation the rights
//! to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//! copies of the Software, and to permit persons to whom the Software is
//! furnished to do so, subject to the following conditions:
//! 
//! The above copyright notice and this permission notice shall be included in
//! all copies or substantial portions of the Software.
//! 
//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//! AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//! THE SOFTWARE.
//! 
//! \section credits Credits
//! Much of the code in this file was originally derived from TinyODBC.<br />
//! TinyODBC is hosted at http://code.google.com/p/tiodbc/<br />
//! Copyright (C) 2008 SqUe squarious@gmail.com<br />
//! License: The MIT License<br />
//! 
//! Transaction support was based on the implementation in SimpleDB: C++ ODBC database API.<br/>
//! SimpleDB is hosted at http://simpledb.sourceforge.net<br/>
//! Copyright (C) 2006 Eminence Technology Pty Ltd<br/>
//! Copyright (C) 2008-2010,2012 Russell Kliese russell@kliese.id.au<br/>
//! License: GNU Lesser General Public version 2.1<br/>
//! 
//! Some improvements and features are based on The Python ODBC Library.<br/>
//! The Python ODBC Library is hosted at http://code.google.com/p/pyodbc/<br/>
//! License: The MIT License<br/>
//! 
//! Implementation of column binding inspired by Nick E. Geht's source code posted to on CodeGuru.<br />
//! GSODBC hosted at http://www.codeguru.com/mfc_database/gsodbc.html<br />
//! Copyright (C) 2002 Nick E. Geht<br />
//! License: Perpetual license to reproduce, distribute, adapt, perform, display, and sublicense.<br/>
//! See http://www.codeguru.com/submission-guidelines.php for details.<br />

//! \page examples Example Usage
//! \brief Example library usage.
//! \include example.cpp

#ifndef NANODBC_H
#define NANODBC_H

#include <stdexcept>
#include <string>

#ifdef NANODBC_USE_BOOST
    #define NANODBC_TR1_STD std::
    #include <boost/smart_ptr.hpp>
    #include <boost/cstdint.hpp>
    namespace std
    {
        using namespace boost;
    }
#else
    // You must explicitly request C++11 support by defining NANODBC_USE_CPP11 at compile time
    // , otherwise nanodbc will assume it must use tr1 instead.
    #ifndef NANODBC_USE_CPP11
        #include <tr1/cstdint>
        #include <tr1/memory>
        #define NANODBC_TR1_STD std::tr1::
    #else
        #include <cstdint>
        #include <memory>
        #define NANODBC_TR1_STD std::
    #endif
#endif

//! \brief The entirety of nanodbc can be found within this one namespace.
//! \todo Implement retrieval of blob data.
//! \todo Implement reflective features for columns, such as type, to enable visitation.
namespace nanodbc
{

// You must explicitly request Unicode support by defining NANODBC_USE_UNICODE at compile time.
#ifndef DOXYGEN
    #ifdef NANODBC_USE_UNICODE
        typedef std::wstring string_type;
    #else
        typedef std::string string_type;
    #endif // NANODBC_USE_UNICODE
#else
    //! string_type will be std::wstring if NANODBC_USE_UNICODE is defined, otherwise std::string.
    typedef unspecified-type string_type;
#endif // DOXYGEN

//! \addtogroup exceptions Exception Types
//! \brief Possible error conditions.
//!
//! Specific errors such as type_incompatible_error, null_access_error, and index_range_error can arise
//! from improper use of the nanodbc library. The general database_error is for all other situations
//! in which the ODBC driver or C API reports an error condition. The explanatory string for database_error
//! will, if possible, contain a diagnostic message obtained from SQLGetDiagRec().
//! @{

//! \brief Type incompatible.
//! \see exceptions
class type_incompatible_error : public std::runtime_error
{
public:
    type_incompatible_error();
    const char* what() const throw();
};

//! \brief Accessed null data.
//! \see exceptions
class null_access_error : public std::runtime_error
{
public:
    null_access_error();
    const char* what() const throw();
};

//! \brief Index out of range.
//! \see exceptions
class index_range_error : public std::runtime_error
{
public:
    index_range_error();
    const char* what() const throw();
};

//! \brief Programming logic error.
//! \see exceptions
class programming_error : public std::runtime_error
{
public:
    explicit programming_error(const std::string& info);
    const char* what() const throw();
};

//! \brief General database error.
//! \see exceptions
class database_error : public std::runtime_error
{
public:
    //! \brief Creates a runtime_error with a message describing the last ODBC error generated for the given handle and handle_type.
    //! \param handle The native ODBC statement or connection handle.
    //! \param handle_type The native ODBC handle type code for the given handle.
    //! \param info Additional information that will be appended to the beginning of the error message.
    database_error(void* handle, short handle_type, const std::string& info = "");
    const char* what() const throw();
};

//! @}

//! \addtogroup utility Utility Classes
//! \brief Additional nanodbc utility classes.
//!
//! \{

//! \brief A type for representing date data.
struct date
{
    NANODBC_TR1_STD int16_t year; //!< Year [0-inf).
    NANODBC_TR1_STD int16_t month; //!< Month of the year [1-12].
    NANODBC_TR1_STD int16_t day; //!< Day of the month [1-31].
};

//! \brief A type for representing timestamp data.
struct timestamp
{
    NANODBC_TR1_STD int16_t year;   //!< Year [0-inf).
    NANODBC_TR1_STD int16_t month;  //!< Month of the year [1-12].
    NANODBC_TR1_STD int16_t day;    //!< Day of the month [1-31].
    NANODBC_TR1_STD int16_t hour;   //!< Hours since midnight [0-23].
    NANODBC_TR1_STD int16_t min;    //!< Minutes after the hour [0-59].
    NANODBC_TR1_STD int16_t sec;    //!< Seconds after the minute.
    NANODBC_TR1_STD int32_t fract;  //!< Fractional seconds.
};

//! \}

//! \addtogroup main Main Classes
//! \brief Main nanodbc classes.
//!
//! @{

//! \brief A resource for managing transaction commits and rollbacks.
//!
//! \attention You will want to use transactions if you are doing batch operations because it will prevent auto commits from occurring after each individual operation is executed.
class transaction
{
public:
    //! \brief Begin a transaction on the given connection object.
    //! \post Operations that modify the database must now be committed before taking effect.
    //! \throws database_error
    explicit transaction(const class connection& conn);

    //! Copy constructor.
    transaction(const transaction& rhs);

    //! Assignment.
    transaction& operator=(transaction rhs);

    //! Member swap.
    void swap(transaction& rhs) throw();

    //! \brief If this transaction has not been committed, will will rollback any modifying operations.
    ~transaction() throw();

    //! \brief Marks this transaction for commit.
    //! \throws database_error
    void commit();

    //! \brief Marks this transaction for rollback.
    void rollback() throw();

    //! Returns the connection object.
    class connection& connection();

    //! Returns the connection object.
    const class connection& connection() const;

    //! Returns the connection object.
    operator class connection&();

    //! Returns the connection object.
    operator const class connection&() const;

private:
    class transaction_impl;
    friend class nanodbc::connection;

private:
    NANODBC_TR1_STD shared_ptr<transaction_impl> impl_;
};

//! \brief Represents a statement on the database.
class statement
{
public:
    //! \brief Creates a new un-prepared statement.
    //! \see execute(), execute_direct(), open(), prepare()
    statement();

    //! \brief Constructs a statement object and associates it to the given connection.
    //! \param conn The connection to use.
    //! \see open(), prepare()
    explicit statement(class connection& conn);

    //! \brief Constructs and prepares a statement using the given connection and query.
    //! \param conn The connection to use.
    //! \param query The SQL query statement.
    //! \see execute(), execute_direct(), open(), prepare()
    statement(class connection& conn, const string_type& query);

    //! Copy constructor.
    statement(const statement& rhs);

    //! Assignment.
    statement& operator=(statement rhs);

    //! Member swap.
    void swap(statement& rhs) throw();

    //! \brief Closes the statement.
    //! \see close()
    ~statement() throw();

    //! \brief Creates a statement for the given connection.
    //! \param conn The connection where the statement will be executed.
    //! \throws database_error
    void open(class connection& conn);

    //! \brief Returns true if connection is open.
    bool open() const;

    //! \brief Returns true if connected to the database.
    bool connected() const;

    //! \brief Returns the associated connection object if any.
    class connection& connection();

    //! \brief Returns the associated connection object if any.
    const class connection& connection() const;

    //! \brief Returns the native ODBC statement handle.
    void* native_statement_handle() const;

    //! \brief Closes the statement and frees all associated resources.
    void close();

    //! \brief Cancels execution of the statement.
    //! \throws database_error
    void cancel();

    //! \brief Opens and prepares the given statement to execute on the given connection.
    //! \param conn The connection where the statement will be executed.
    //! \param query The SQL query that will be executed.
    //! \see open()
    //! \throws database_error
    void prepare(class connection& conn, const string_type& query);

    //! \brief Prepares the given statement to execute its associated connection.
    //! If the statement is not open throws programming_error.
    //! \param conn The connection where the statement will be executed.
    //! \param query The SQL query that will be executed.
    //! \see open()
    //! \throws database_error, programming_error
    void prepare(const string_type& query);

    //! \brief Immediately opens, prepares, and executes the given query directly on the given connection.
    //! \param conn The connection where the statement will be executed.
    //! \param query The SQL query that will be executed.
    //! \param batch_operations Numbers of rows to fetch per rowset, or the number of batch parameters to process.
    //! \return A result set object.
    //! \attention You will want to use transactions if you are doing batch operations because it will prevent auto commits from occurring after each individual operation is executed.
    //! \see open(), prepare(), execute(), result, transaction
    class result execute_direct(class connection& conn, const string_type& query, long batch_operations = 1);

    //! \brief Execute the previously prepared query now.
    //! \param batch_operations Numbers of rows to fetch per rowset, or the number of batch parameters to process.
    //! \throws database_error
    //! \return A result set object.
    //! \attention You will want to use transactions if you are doing batch operations because it will prevent auto commits from occurring after each individual operation is executed.
    //! \see open(), prepare(), execute(), result, transaction
    class result execute(long batch_operations = 1);

    //! \brief Returns the number of rows affected by the request or –1 if the number of affected rows is not available.
    //! \throws database_error
    long affected_rows() const;

    //! \brief Returns the number of columns in a result set.
    //! \throws database_error
    short columns() const;

    //! \brief Resets all currently bound parameters.
    void reset_parameters() throw();

    //! \brief Returns the parameter size for the indicated parameter placeholder within a prepared statement.
    unsigned long parameter_size(long param) const;

    enum param_direction { In, Out, InOut, Return };

    //! \brief Binds the given value to the given parameter placeholder number in the prepared statement.
    //!
    //! If your prepared SQL query has any ? placeholders, this is how you bind values to them.
    //! Placeholder numbers count from left to right and are 0-indexed.
    //! 
    //! \param param Placeholder position.
    //! \param value Value to substitute into placeholder.
    //! \param nulls Used to batch insert nulls into the database.
    //! \throws database_error
    template<class T>
    void bind_parameter(long param, const T* value, long* nulls = 0, param_direction direction = In);

    //! \brief Binds the given values to the given parameter placeholder number in the prepared statement.
    //!
    //! If your prepared SQL query has any ? placeholders, this is how you bind values to them.
    //! Placeholder numbers count from left to right and are 0-indexed.
    //! 
    //! Typically you would use bulk operations with a row size of N when executing a statement bound this way.
    //! 
    //! \param param Placeholder position.
    //! \param values Values to bulk substitute into placeholder.
    //! \throws database_error
    template<class T, std::size_t N>
    void bind_parameter(long param, const T(*values)[N])
    {
        bind_parameter(param, reinterpret_cast<const T*>(values));
    }

private:
    class statement_impl;
    friend class nanodbc::result;

private:
    NANODBC_TR1_STD shared_ptr<statement_impl> impl_;
};

//! \brief Manages and encapsulates ODBC resources such as the connection and environment handles.
class connection
{
public:
    //! \brief Create new connection object, initially not connected.
    connection();

    //! Copy constructor.
    connection(const connection& rhs);

    //! Assignment.
    connection& operator=(connection rhs);

    //! Member swap.
    void swap(connection&) throw();

    //! \brief Create new connection object and immediately connect to the given data source.
    //! \param dsn The name of the data source.
    //! \param user The username for authenticating to the data source.
    //! \param pass The password for authenticating to the data source.
    //! \param timeout The number in seconds before connection timeout.
    //! \throws database_error
    //! \see connected(), connect()
    connection(const string_type& dsn, const string_type& user, const string_type& pass, long timeout = 5);

    //! \brief Create new connection object and immediately connect using the given connection string.
    //! \param connection_string The connection string for establishing a connection.
    //! \throws database_error
    //! \see connected(), connect()
    connection(const string_type& connection_string, long timeout = 5);

    //! \brief Automatically disconnects from the database and frees all associated resources.
    ~connection() throw();

    //! \brief Create new connection object and immediately connect to the given data source.
    //! \param dsn The name of the data source.
    //! \param user The username for authenticating to the data source.
    //! \param pass The password for authenticating to the data source.
    //! \param timeout The number in seconds before connection timeout.
    //! \throws database_error
    //! \see connected()
    void connect(const string_type& dsn, const string_type& user, const string_type& pass, long timeout = 5);

    //! \brief Create new connection object and immediately connect using the given connection string.
    //! \param connection_string The connection string for establishing a connection.
    //! \param timeout The number in seconds before connection timeout.
    //! \throws database_error
    //! \see connected()
    void connect(const string_type& connection_string, long timeout = 5);

    //! \brief Returns true if connected to the database.
    bool connected() const;

    //! \brief Disconnects from the database, but maintains environment and handle resources.
    void disconnect() throw();

    //! \brief Returns the number of transactions currently held for this connection.
    std::size_t transactions() const;

    //! \brief Returns the native ODBC database connection handle.
    void* native_dbc_handle() const;

    //! \brief Returns the native ODBC environment handle.
    void* native_env_handle() const;

    //! \brief Returns the name of the ODBC driver.
    //! \throws database_error
    string_type driver_name() const;

private:
    std::size_t ref_transaction();
    std::size_t unref_transaction();
    bool rollback() const;
    void rollback(bool onoff);

private:
    class connection_impl;
    friend class nanodbc::transaction::transaction_impl;

private:
    NANODBC_TR1_STD shared_ptr<connection_impl> impl_;
};

//! \brief A resource for managing result sets from statement execution.
//!
//! \see statement::execute(), statement::execute_direct()
//! \note result objects may be copied, however all copies will refer to the same underlying ODBC result set.
class result
{
public:
    //! Empty result set.
    result();

    //! Free result set.
    ~result() throw();

    //! Copy constructor.
    result(const result& rhs);

    //! Assignment.
    result& operator=(result rhs);

    //! Member swap.
    void swap(result& rhs) throw();

    //! \brief Returns the native ODBC statement handle.
    void* native_statement_handle() const;

    //! \brief The rowset size for this result set.
    long rowset_size() const throw();

    //! \brief Returns the number of rows affected by the request or –1 if the number of affected rows is not available.
    //! \throws database_error
    long affected_rows() const;

    //! \brief Returns the number of rows in the current rowset or 0 if the number of rows is not available.
    long rows() const throw();

    //! \brief Returns the number of columns in a result set.
    //! \throws database_error
    short columns() const;

    //! \brief Fetches the first row in the current result set.
    //! \return true if there are more results or false otherwise.
    //! \throws database_error
    bool first();

    //! \brief Fetches the last row in the current result set.
    //! \return true if there are more results or false otherwise.
    //! \throws database_error
    bool last();

    //! \brief Fetches the next row in the current result set.
    //! \return true if there are more results or false otherwise.
    //! \throws database_error
    bool next();

    //! \brief Fetches the prior row in the current result set.
    //! \return true if there are more results or false otherwise.
    //! \throws database_error
    bool prior();

    //! \brief Moves to and fetches the specified row in the current result set.
    //! \return true if there are results or false otherwise.
    //! \throws database_error
    bool move(long row);

    //! \brief Skips a number of rows and then fetches the resulting row in the current result set.
    //! \return true if there are results or false otherwise.
    //! \throws database_error
    bool skip(long rows);

    //! \brief Returns the row position in the current result set.
    unsigned long position() const;

    //! \brief Returns true if there are no more results in the current result set.
    bool end() const throw();

    //! \brief Gets data from the given column in the selected row of the current rowset.
    //!
    //! Columns are numbered from left to right and 0-indexed.
    //! \param Column position. 
    //! \param row If there are multiple rows in this rowset, get from the specified row.
    //! \throws database_error, index_range_error, type_incompatible_error, null_access_error
    template<class T>
    T get(short column) const;

    //! \brief Gets data from the given column in the selected row of the current rowset.
    //! If the data is null, fallback is returned instead.
    //!
    //! Columns are numbered from left to right and 0-indexed.
    //! \param Column position. 
    //! \param row If there are multiple rows in this rowset, get from the specified row.
    //! \throws database_error, index_range_error, type_incompatible_error
    template<class T>
    T get(short column, const T& fallback) const;

    //! \brief Returns true if and only if the given column in the selected row of the current rowset is null.
    //!
    //! Columns are numbered from left to right and 0-indexed.
    //! \param Column position. 
    //! \param row If there are multiple rows in this rowset, get from the specified row.
    //! \throws database_error, index_range_error
    bool is_null(short column) const;

    //! \brief Returns the name of the specified column.
    //!
    //! Columns are numbered from left to right and 0-indexed.
    //! \param Column position. 
    //! \throws index_range_error
    string_type column_name(short column) const;

    //! Returns a identifying integer value representing the C type of this column.
    int column_datatype(short column) const;

    //! Returns the next result, for example when stored procedure returns multiple result sets.
    bool next_result() const;

private:
    result(statement statement, long rowset_size);

private:
    class result_impl;
    friend class nanodbc::statement::statement_impl;

private:
    NANODBC_TR1_STD shared_ptr<result_impl> impl_;
};

//! @}

//! \addtogroup main Free Functions
//! \brief Convenience functions.
//!
//! @{

//! \brief Immediately opens, prepares, and executes the given query directly on the given connection.
//! \param conn The connection where the statement will be executed.
//! \param query The SQL query that will be executed.
//! \param batch_operations Numbers of rows to fetch per rowset, or the number of batch parameters to process.
//! \return A result set object.
//! \attention You will want to use transactions if you are doing batch operations because it will prevent auto commits from occurring after each individual operation is executed.
//! \see open(), prepare(), execute(), result, transaction
result execute(connection& conn, const string_type& query, long batch_operations = 1);

//! \brief Execute the previously prepared query now.
//! \param stmt The prepared statement that will be executed.
//! \param batch_operations Numbers of rows to fetch per rowset, or the number of batch parameters to process.
//! \throws database_error
//! \return A result set object.
//! \attention You will want to use transactions if you are doing batch operations because it will prevent auto commits from occurring after each individual operation is executed.
//! \see open(), prepare(), execute(), result
result execute(statement& stmt, long batch_operations = 1);

//! \brief Execute the previously prepared query now.
//! Executes within the context of a transaction object and commits the transaction directly after execution.
//! \param stmt The prepared statement that will be executed in batch.
//! \param batch_operations Numbers of rows to fetch per rowset, or the number of batch parameters to process.
//! \throws database_error
//! \return A result set object.
//! \see open(), prepare(), execute(), result, transaction
result transact(statement& stmt, long batch_operations);

//! \brief Prepares the given statement to execute on it associated connection.
//! If the statement is not open throws programming_error.
//! \param conn The connection where the statement will be executed.
//! \param query The SQL query that will be executed.
//! \see open()
//! \throws database_error, programming_error
void prepare(statement& stmt, const string_type& query);

//! @}

} // namespace nanodbc

#undef NANODBC_TR1_STD

#endif // NANODBC_H
