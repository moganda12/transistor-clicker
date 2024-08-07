#include <gmpxx.h>
#include <vector>
#include <string>
#include <math.h>
#include <sstream>

namespace GRP {
	using integer = mpz_class;
	using flat = mpf_class;
	using number = mpq_class;

	using str = std::string;

	number pow(number base, integer exp) {
		number result = 1;
		for (integer i = 0; i < exp; i++)
			result *= base;
		return result;
	}

	integer precision;
	integer kilo = 1'000;

	std::vector<str> formatLong = {" thousand"," million"," billion"," trillion"," quadrillion"," quintillion"," sextillion"," septillion"," octillion"," nonillion"};
	std::vector<str> prefixes = {"","un","duo","tre","quattuor","quin","sex","septen","octo","novem"};
	std::vector<str> suffixes = {"decillion","vigintillion","trigintillion","quadragintillion","quinquagintillion","sexagintillion","septuagintillion","octogintillion","nonagintillion"};

	integer round(number x) {
		return  x.get_num() / x.get_den();
	}

	str toString(number x) {
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

			integer big = round(x / kilo);
			integer small = round(x - big * kilo);

			if(small == 0) {
				result << big << formatLong[suffix];
			} else {
				str sml = small.get_str();
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
			x *= 10;
			integer big = round(x/10);

			integer small = round(x - big * 10);

			integer kilos = big / kilo;
			integer ones = big - kilos * kilo;
			if(kilos > 0) {
				str oness = ones.get_str();
				char onesf[] = "000";
				for(size_t i = 0; i < oness.length(); i++) {
					onesf[2 - i] = oness[oness.length() - i - 1];
				}
				
				result << kilos << ',' << onesf;
			} else {
				if(small != 0) result << ones << '.' << small;
				else result << ones;
			}

			return result.str();
		}

		return "error: HOW!";
	}

	void init(integer precision = 1) {
		precision = pow(precision, 10);

		for(int i = 0; i < suffixes.size(); i++) {
			for(int j = 0; j < prefixes.size(); j++) {
				formatLong.push_back(' ' + prefixes[j] + suffixes[i]);
			}
		}
	}
}