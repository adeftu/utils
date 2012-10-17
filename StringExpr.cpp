#include "StringExpr.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/variant.hpp>
#include <boost/version.hpp>

#define foreach BOOST_FOREACH

#if BOOST_VERSION >= 103800
    #include <boost/spirit/include/classic_core.hpp>
    #include <boost/spirit/include/classic_confix.hpp>
    #include <boost/spirit/include/classic_escape_char.hpp>
    #include <boost/spirit/include/classic_multi_pass.hpp>
    #include <boost/spirit/include/classic_position_iterator.hpp>
    using namespace boost::spirit::classic;
#else
    #include <boost/spirit/core.hpp>
    #include <boost/spirit/utility/confix.hpp>
    #include <boost/spirit/utility/escape_char.hpp>
    #include <boost/spirit/iterator/multi_pass.hpp>
    #include <boost/spirit/iterator/position_iterator.hpp>
    using namespace boost::spirit;
#endif

#include <stdexcept>
#include <sstream>


using namespace std;

namespace oneandone {
namespace nms {

struct variable
{
    std::string name;
    variable(string n) : name(n) {}
};

typedef boost::variant<std::string, variable> node;


struct StringExprPrivate
{
    std::vector<node> parsed;

    void add_string(const char *first, const char *last) {
        parsed.push_back(string(first, last));
    }
    void add_variable(const char *first, const char *last) {
        parsed.push_back(variable(string(first, last)));
    }
};


StringExpr::StringExpr(const string& expr) : 
    string_parse_(new StringExprPrivate),
    string_expr_(expr)
{
    using boost::bind;
    boost::function<void(const char *, const char *)> add_string, add_variable;
    add_string = bind(&StringExprPrivate::add_string, string_parse_.get(), _1, _2);
    add_variable = bind(&StringExprPrivate::add_variable, string_parse_.get(), _1, _2);

    bool success = parse(expr.c_str(), *(
        confix_p("${", (+anychar_p) [add_variable], "}")
        | ((+(anychar_p - "${")) [add_string])
    )).full;
    if (!success) {
        throw runtime_error("Parsing " + expr + " as a string_expr failed");
    }
}


StringExpr::~StringExpr() {}


struct eval_data : public boost::static_visitor<void>
{
    std::stringstream result;
    const map<string, string> &vars;
    eval_data(const map<string, string> &v) : vars(v) {}

    void operator()(const std::string& node)
    {
        result << node;
    }

    void operator()(const variable& node)
    {
        map<string, string>::const_iterator it_var = vars.find(node.name);
        if (it_var != vars.end())
            result << it_var->second;
    }
};


const std::string StringExpr::Eval(const std::map<string, string>& vars) const
{
    eval_data ret(vars);
    foreach (const node &n, string_parse_->parsed) {
        boost::apply_visitor(ret, n);
    }
    return ret.result.str();
}


struct vars_data : public boost::static_visitor<void>
{
    std::set<std::string> vars;

    void operator()(const std::string& node)
    {
    }

    void operator()(const variable& node)
    {
        vars.insert(node.name);
    }
};


const std::set<std::string> StringExpr::GetVars() const
{
    vars_data vars_collector;
    foreach (const node &n, string_parse_->parsed) {
        boost::apply_visitor(vars_collector, n);
    }
    return vars_collector.vars;
}


}
}