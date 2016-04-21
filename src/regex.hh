#ifndef regex_hh_INCLUDED
#define regex_hh_INCLUDED

#include "string.hh"
#include "exception.hh"
#include "utf8_iterator.hh"

#define KAK_USE_STDREGEX

#ifdef KAK_USE_STDREGEX
#include <regex>
namespace regex_ns = std;
#else
#include <boost/regex.hpp>
namespace regext_ns = boost;
#endif

namespace Kakoune
{

struct regex_error : runtime_error
{
    regex_error(StringView desc)
        : runtime_error{format("regex error: '{}'", desc)}
    {}
};

#ifdef KAK_USE_STDREGEX
using RegexTraits = std::regex_traits<wchar_t>;
#else
using RegexTraits = boost::c_regex_traits<wchar_t>;
#endif

using RegexBase = regex_ns::basic_regex<wchar_t, RegexTraits>;

// Regex that keeps track of its string representation
struct Regex : RegexBase
{
    Regex() = default;

    explicit Regex(StringView re, flag_type flags = ECMAScript);
    bool empty() const { return m_str.empty(); }
    bool operator==(const Regex& other) const { return m_str == other.m_str; }
    bool operator!=(const Regex& other) const { return m_str != other.m_str; }

    const String& str() const { return m_str; }

    static constexpr const char* option_type_name = "regex";

private:
    String m_str;
};

template<typename It>
using RegexUtf8It = utf8::iterator<It, wchar_t, ssize_t>;

template<typename It>
using RegexIteratorBase = regex_ns::regex_iterator<RegexUtf8It<It>, wchar_t,
                                                   RegexTraits>;

namespace RegexConstant = regex_ns::regex_constants;

template<typename Iterator>
struct MatchResults : regex_ns::match_results<RegexUtf8It<Iterator>>
{
    using ParentType = regex_ns::match_results<RegexUtf8It<Iterator>>;
    struct SubMatch : std::pair<Iterator, Iterator>
    {
        SubMatch() = default;
        SubMatch(const regex_ns::sub_match<RegexUtf8It<Iterator>>& m)
            : std::pair<Iterator, Iterator>{m.first.base(), m.second.base()},
              matched{m.matched}
        {}

        bool matched = false;
    };

    struct iterator : regex_ns::match_results<RegexUtf8It<Iterator>>::iterator
    {
        using ParentType = typename regex_ns::match_results<RegexUtf8It<Iterator>>::iterator;
        iterator(const ParentType& it) : ParentType(it) {}

        SubMatch operator*() const { return {ParentType::operator*()}; }
    };

    iterator begin() const { return {ParentType::begin()}; }
    iterator cbegin() const { return {ParentType::cbegin()}; }
    iterator end() const { return {ParentType::end()}; }
    iterator cend() const { return {ParentType::cend()}; }

    SubMatch operator[](size_t s) const { return {ParentType::operator[](s)}; }
};

template<typename Iterator>
struct RegexIterator : RegexIteratorBase<Iterator>
{
    using Utf8It = RegexUtf8It<Iterator>;
    using ValueType = MatchResults<Iterator>;

    RegexIterator() = default;
    RegexIterator(Iterator begin, Iterator end, const Regex& re,
                  RegexConstant::match_flag_type flags = RegexConstant::match_default)
        : RegexIteratorBase<Iterator>{Utf8It{begin, begin, end}, Utf8It{end, begin, end}, re, flags} {}

    const ValueType& operator*() const { return *reinterpret_cast<const ValueType*>(&RegexIteratorBase<Iterator>::operator*()); }
    const ValueType* operator->() const { return reinterpret_cast<const ValueType*>(RegexIteratorBase<Iterator>::operator->()); }
};

inline RegexConstant::match_flag_type match_flags(bool bol, bool eol, bool bow, bool eow)
{
    return (bol ? RegexConstant::match_default : RegexConstant::match_not_bol) |
           (eol ? RegexConstant::match_default : RegexConstant::match_not_eol) |
           (bow ? RegexConstant::match_default : RegexConstant::match_not_bow)/* |
           (eow ? RegexConstant::match_default : RegexConstant::match_not_eow)*/;
}

template<typename It>
bool regex_match(It begin, It end, const Regex& re)
{
    using Utf8It = RegexUtf8It<It>;
    try
    {
        return regex_ns::regex_match(Utf8It{begin, begin, end}, Utf8It{end, begin, end}, re);
    }
    catch (std::runtime_error& err)
    {
        throw runtime_error{format("Regex matching error: {}", err.what())};
    }
}

template<typename It>
bool regex_match(It begin, It end, MatchResults<It>& res, const Regex& re)
{
    using Utf8It = RegexUtf8It<It>;
    try
    {
        return regex_ns::regex_match(Utf8It{begin, begin, end}, Utf8It{end, begin, end}, res, re);
    }
    catch (std::runtime_error& err)
    {
        throw runtime_error{format("Regex matching error: {}", err.what())};
    }
}

template<typename It>
bool regex_search(It begin, It end, const Regex& re,
                  RegexConstant::match_flag_type flags = RegexConstant::match_default)
{
    using Utf8It = RegexUtf8It<It>;
    try
    {
        return regex_ns::regex_search(Utf8It{begin, begin, end}, Utf8It{end, begin, end}, re);
    }
    catch (std::runtime_error& err)
    {
        throw runtime_error{format("Regex searching error: {}", err.what())};
    }
}

template<typename It>
bool regex_search(It begin, It end, MatchResults<It>& res, const Regex& re,
                  RegexConstant::match_flag_type flags = RegexConstant::match_default)
{
    using Utf8It = RegexUtf8It<It>;
    try
    {
        return regex_ns::regex_search(Utf8It{begin, begin, end}, Utf8It{end, begin, end}, res, re);
    }
    catch (std::runtime_error& err)
    {
        throw runtime_error{format("Regex searching error: {}", err.what())};
    }
}

String option_to_string(const Regex& re);
void option_from_string(StringView str, Regex& re);

}

#endif // regex_hh_INCLUDED
