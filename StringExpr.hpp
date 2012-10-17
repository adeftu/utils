#ifndef STRINGEXPR_HPP
#define STRINGEXPR_HPP

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>
#include <set>
#include <string>

namespace oneandone {
namespace nms {

struct StringExprPrivate;

class StringExpr : boost::noncopyable
{
public:
    // TODO this constructor should also take the list of available variable names for validation
    // purposes
    StringExpr(const std::string &expr);
    ~StringExpr();
    const std::string Eval(const std::map<std::string, std::string> &vars) const;
    const std::set<std::string> GetVars() const;
    std::string GetStringExpr() const { return string_expr_; }

private:
    boost::scoped_ptr<StringExprPrivate> string_parse_;
    std::string string_expr_;
};

}   // namespace oneandone
}   // namespace nms

#endif // STRINGEXPR_HPP
