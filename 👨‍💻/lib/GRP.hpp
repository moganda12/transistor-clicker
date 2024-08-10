#include <gmpxx.h>
#include <vector>
#include <string>
#include <math.h>
#include <sstream>

namespace GRP {
	
	mpq_class pow(mpq_class base, mpz_class exp) {
		mpq_class result = 1;
		for (mpz_class i = 0; i < exp; i++)
			result *= base;
		return result;
	}
	
	mpz_class kilo = 1'000;

	std::vector<std::string> formatLong = {" thousand"," million"," billion"," trillion"," quadrillion"," quintillion"," sextillion"," septillion"," octillion"," nonillion"};
	std::vector<std::string> prefixes = {"","un","duo","tre","quattuor","quin","sex","septen","octo","novem"};
	std::vector<std::string> suffixes = {"decillion","vigintillion","trigintillion","quadragintillion","quinquagintillion","sexagintillion","septuagintillion","octogintillion","nonagintillion"};

	mpz_class round(mpq_class x) {
		return  x.get_num() / x.get_den();
	}

	mpz_class operator%(mpz_class a, mpz_class b) {
		return a - (a / b) * b;
	}

	std::string toString(mpq_class x, mpz_class precision = 1) {
		std::stringstream result;
		if (x >= 1'000'000) {
			size_t suffix = 1;

			x /= kilo;
			x = round(x);

			while(x > 999'999) {
				x /= kilo;
				x = round(x);
				suffix++;
			}

			mpz_class big = round(x / kilo);
			mpz_class small = round(x - big * kilo);

			if(small == 0) {
				result << big << formatLong[suffix];
			} else {
				std::string sml = small.get_str();
				char smallf[] = "000";
				for(int i = 0; i < sml.length(); i++) {
					smallf[2 - i] = sml[sml.length() - i - 1];
				}

				if(smallf[1] == '0' && smallf[2] == '0') smallf[1] = 0x00;
				if(smallf[2] == '0') smallf[2] = 0x00;

				result << big << '.' << smallf << formatLong[suffix];
			}

			return result.str();
		} else {
			mpz_class big = round(x);

			std::string small = "";
			std::string smallBuf = "";
			bool smallHasDigits = false;

			for(mpz_class i = 1; i <= precision; i++) {
				mpz_class digit = round(x * pow(10, i)) % 10;
				smallBuf.append(digit.get_str());
				if(digit != 0) {
					smallHasDigits = true;
					small.append(smallBuf);
				}
			}

			mpz_class kilos = big / kilo;
			mpz_class ones = big - kilos * kilo;
			if(kilos > 0) {
				std::string oness = ones.get_str();
				char onesf[] = "000";
				for(size_t i = 0; i < oness.length(); i++) {
					onesf[2 - i] = oness[oness.length() - i - 1];
				}
				
				result << kilos << ',' << onesf;
			} else {
				if(smallHasDigits) result << ones << '.' << small;
				else result << ones;
			}

			return result.str();
		}

		return "error: HOW!";
	}

	void init() {
		for(int i = 0; i < suffixes.size(); i++) {
			for(int j = 0; j < prefixes.size(); j++) {
				formatLong.push_back(' ' + prefixes[j] + suffixes[i]);
			}
		}
	}
}