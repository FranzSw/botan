/*
* SCAN Name Abstraction
* (C) 2008,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SCAN_NAME_H_
#define BOTAN_SCAN_NAME_H_

#include <botan/types.h>
#include <string>
#include <string_view>
#include <vector>

namespace Botan {

/**
A class encapsulating a SCAN name (similar to JCE conventions)
http://www.users.zetnet.co.uk/hopwood/crypto/scan/
*/
class SCAN_Name final {
   public:
      /**
      * Create a SCAN_Name
      * @param algo_spec A SCAN-format name
      */
      explicit SCAN_Name(const char* algo_spec);

      /**
      * Create a SCAN_Name
      * @param algo_spec A SCAN-format name
      */
      explicit SCAN_Name(std::string_view algo_spec);

      /**
      * @return original input string
      */
      const std::string to_string() const { return m_orig_algo_spec; }

      /**
      * @return algorithm name
      */
      const std::string algo_name() const { return m_alg_name; }

      /**
      * @return number of arguments
      */
      size_t arg_count() const { return m_args.size(); }

      /**
      * @param lower is the lower bound
      * @param upper is the upper bound
      * @return if the number of arguments is between lower and upper
      */
      bool arg_count_between(size_t lower, size_t upper) const {
         return ((arg_count() >= lower) && (arg_count() <= upper));
      }

      /**
      * @param i which argument
      * @return ith argument
      */
      std::string arg(size_t i) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument or the default value
      */
      std::string arg(size_t i, std::string_view def_value) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument as an integer, or the default value
      */
      size_t arg_as_integer(size_t i, size_t def_value) const;

      /**
      * @param i which argument
      * @return ith argument as an integer
      */
      size_t arg_as_integer(size_t i) const;

      /**
      * @return cipher mode (if any)
      */
      std::string cipher_mode() const { return (m_mode_info.size() >= 1) ? m_mode_info[0] : ""; }

      /**
      * @return cipher mode padding (if any)
      */
      std::string cipher_mode_pad() const { return (m_mode_info.size() >= 2) ? m_mode_info[1] : ""; }

   private:
      std::string m_orig_algo_spec;
      std::string m_alg_name;
      std::vector<std::string> m_args;
      std::vector<std::string> m_mode_info;
};

// This is unrelated but it is convenient to stash it here
template <typename T>
std::vector<std::string> probe_providers_of(std::string_view algo_spec,
                                            const std::vector<std::string>& possible = {"base"}) {
   std::vector<std::string> providers;
   for(auto&& prov : possible) {
      auto o = T::create(algo_spec, prov);
      if(o) {
         providers.push_back(prov);  // available
      }
   }
   return providers;
}

}  // namespace Botan

#endif
