/**
 *  @file
 *  test fredlib utils
 */

#ifndef TESTS_FREDLIB_UTIL_69498451224
#define TESTS_FREDLIB_UTIL_69498451224

static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}

#endif // #include guard end
