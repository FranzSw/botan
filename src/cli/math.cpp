/*
* (C) 2009,2010,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "cli.h"

#if defined(BOTAN_HAS_NUMBERTHEORY)

#include <botan/reducer.h>
#include <botan/numthry.h>
#include <botan/monty.h>
#include <iterator>

namespace Botan_CLI {

class Modular_Inverse final : public Command
   {
   public:
      Modular_Inverse() : Command("mod_inverse n mod") {}

      std::string group() const override
         {
         return "numtheory";
         }

      std::string description() const override
         {
         return "Calculates a modular inverse";
         }

      void go() override
         {
         const Botan::BigInt n(get_arg("n"));
         const Botan::BigInt mod(get_arg("mod"));

         output() << Botan::inverse_mod(n, mod) << "\n";
         }
   };

BOTAN_REGISTER_COMMAND("mod_inverse", Modular_Inverse);

class Gen_Prime final : public Command
   {
   public:
      Gen_Prime() : Command("gen_prime --count=1 bits") {}

      std::string group() const override
         {
         return "numtheory";
         }

      std::string description() const override
         {
         return "Samples one or more primes";
         }

      void go() override
         {
         const size_t bits = get_arg_sz("bits");
         const size_t cnt = get_arg_sz("count");

         for(size_t i = 0; i != cnt; ++i)
            {
            const Botan::BigInt p = Botan::random_prime(rng(), bits);
            output() << p << "\n";
            }
         }
   };

BOTAN_REGISTER_COMMAND("gen_prime", Gen_Prime);

class Is_Prime final : public Command
   {
   public:
      Is_Prime() : Command("is_prime --prob=56 n") {}

      std::string group() const override
         {
         return "numtheory";
         }

      std::string description() const override
         {
         return "Test if the integer n is composite or prime";
         }

      void go() override
         {
         Botan::BigInt n(get_arg("n"));
         const size_t prob = get_arg_sz("prob");
         const bool prime = Botan::is_prime(n, rng(), prob);

         output() << n << " is " << (prime ? "probably prime" : "composite") << "\n";
         }
   };

BOTAN_REGISTER_COMMAND("is_prime", Is_Prime);

/*
* Factor integers using a combination of trial division by small
* primes, and Pollard's Rho algorithm
*/
class Factor final : public Command
   {
   public:
      Factor() : Command("factor n") {}

      std::string group() const override
         {
         return "numtheory";
         }

      std::string description() const override
         {
         return "Factor a given integer";
         }

      void go() override
         {
         Botan::BigInt n(get_arg("n"));

         std::vector<Botan::BigInt> factors = factorize(n, rng());
         std::sort(factors.begin(), factors.end());

         output() << n << ": ";
         std::copy(factors.begin(), factors.end(), std::ostream_iterator<Botan::BigInt>(output(), " "));
         output() << std::endl;
         }

   private:

      std::vector<Botan::BigInt> factorize(const Botan::BigInt& n_in,
                                           Botan::RandomNumberGenerator& rng)
         {
         Botan::BigInt n = n_in;
         std::vector<Botan::BigInt> factors = remove_small_factors(n);

         while(n != 1)
            {
            if(Botan::is_prime(n, rng))
               {
               factors.push_back(n);
               break;
               }

            Botan::BigInt a_factor = 0;
            while(a_factor == 0)
               {
               a_factor = rho(n, rng);
               }

            std::vector<Botan::BigInt> rho_factored = factorize(a_factor, rng);
            for(size_t j = 0; j != rho_factored.size(); j++)
               {
               factors.push_back(rho_factored[j]);
               }

            n /= a_factor;
            }

         return factors;
         }

      /*
      * Pollard's Rho algorithm, as described in the MIT algorithms book.
      * Uses Brent's cycle finding
      */
      Botan::BigInt rho(const Botan::BigInt& n, Botan::RandomNumberGenerator& rng)
         {
         auto monty_n = std::make_shared<Botan::Montgomery_Params>(n);

         const Botan::Montgomery_Int one(monty_n, monty_n->R1(), false);

         Botan::Montgomery_Int x(monty_n, Botan::BigInt::random_integer(rng, 2, n - 3), false);
         Botan::Montgomery_Int y = x;
         Botan::Montgomery_Int z = one;
         Botan::BigInt d;
         Botan::Montgomery_Int t(monty_n);

         Botan::secure_vector<Botan::word> ws;

         size_t i = 1, k = 2;

         while(true)
            {
            i++;

            if(i >= 0xFFFF0000) // bad seed? too slow? bail out
               {
               break;
               }

            x.square_this(ws); // x = x^2
            x.add(one, ws);

            t = y;
            t -= x;

            z.mul_by(t, ws);

            if(i == k || i % 128 == 0)
               {
               d = Botan::gcd(z.value(), n);
               z = one;

               if(d == n)
                  {
                  // TODO Should rewind here
                  break;
                  }

               if(d != 1)
                  return d;
               }

            if(i == k)
               {
               y = x;
               k = 2 * k;
               }
            }

         // failed
         return 0;
         }

      // Remove (and return) any small (< 2^16) factors
      std::vector<Botan::BigInt> remove_small_factors(Botan::BigInt& n)
         {
         std::vector<Botan::BigInt> factors;

         while(n.is_even())
            {
            factors.push_back(2);
            n /= 2;
            }

         for(size_t j = 0; j != Botan::PRIME_TABLE_SIZE; j++)
            {
            uint16_t prime = Botan::PRIMES[j];
            if(n < prime)
               {
               break;
               }

            Botan::BigInt x = Botan::gcd(n, prime);

            if(x != 1)
               {
               n /= x;

               while(x != 1)
                  {
                  x /= prime;
                  factors.push_back(prime);
                  }
               }
            }

         return factors;
         }
   };

BOTAN_REGISTER_COMMAND("factor", Factor);

}

#endif
