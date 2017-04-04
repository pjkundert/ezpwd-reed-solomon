//  Copyright (c) 2010 Peter Schueller
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <istream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace classic = boost::spirit::classic;

// parse list of doubles from input stream
// throw exception (perhaps including filename) on errors
std::vector<double> parse(std::istream& input, const std::string& filename);

// main function
int main(int, char**)
{
      try
      {
	  for ( auto &&r : parse(std::cin, "STDIN"))
	      std::cout << r << std::endl;
      }
      catch(const std::exception& e)
      {
	  std::cerr << "Exception: " << e.what() << std::endl;
	  return -1;
      }
      return 0;
}

// implementation
std::vector<double> parse(std::istream& input, const std::string& filename)
{
    // iterate over stream input
    typedef std::istreambuf_iterator<char> base_iterator_type;
    base_iterator_type in_begin(input);

    // convert input iterator to forward iterator, usable by spirit parser
    typedef boost::spirit::multi_pass<base_iterator_type> forward_iterator_type;
    forward_iterator_type fwd_begin = boost::spirit::make_default_multi_pass(in_begin);
    forward_iterator_type fwd_end;

    // wrap forward iterator with position iterator, to record the position
    typedef classic::position_iterator2<forward_iterator_type> pos_iterator_type;
    pos_iterator_type position_begin(fwd_begin, fwd_end, filename);
    pos_iterator_type position_end;

    // prepare output
    std::vector<double> output;

    // parse
      try
      {
	  qi::phrase_parse(
			   position_begin, position_end,                               // iterators over input
			   qi::double_ > *(',' > qi::double_) >> qi::eoi,              // recognize list of doubles
			   ascii::space | '#' >> *(ascii::char_ - qi::eol) >> qi::eol, // comment skipper
			   output);                                                    // doubles are stored into this object
      }
      catch(const qi::expectation_failure<pos_iterator_type>& e)
      {
	  const classic::file_position_base<std::string>& pos = e.first.get_position();
	  std::stringstream msg;
	  msg <<
	      "parse error at file " << pos.file <<
	      " line " << pos.line << " column " << pos.column << std::endl <<
	      "'" << e.first.get_currentline() << "'" << std::endl <<
	      std::setw(pos.column) << " " << "^- here";
	  throw std::runtime_error(msg.str());
      }

      // return result
      return output;
}
