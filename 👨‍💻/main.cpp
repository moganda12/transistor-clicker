#include <gmpxx.h>
#include <math.h>
#include <sstream>

#include "./lib/CMD.hpp"
#include "./lib/GRP.hpp"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

using str = std::string;

using integer = mpz_class;
using flat = mpf_class;
using number = mpq_class;

std::hash<str> hasher;

long hash(str s) {
	return hasher(s);
}

enum Buildings {
    cursor = -5238174239041902407,
    moss = 7569504340697116271,
    smallFAB = -2000947298636880309,
    mediumFAB = -1712167728928864321,
    largeFAB = -7940484003211731860,
    i860 = -3460087220543975172,
    startup = 9083628158375851998,
    oaktree = 6580827420282859339
};

struct TclickerSave {
	number transistorBalance;
	number totalTransistors;
	/*integer ascensionCount;
	number ascentionLevels;
	number heavenlyMicrochips;*/
	integer clicks; 
    integer cusors, moss, smallFABs, mediumFABs, largeFABs;
	//integer i860s, startups, oakTrees;
    bool cursorUnlocked, mossUnlocked, smallFABUnlocked, mediumFABUnlocked, largeFABUnlocked;
    //bool i860Unlocked, startupUnlocked, oakTreeUnlocked;
};

struct Trigger {
    CMD::Condition condition;
    CMD::Result result;
};

std::vector<Trigger> triggers;

TclickerSave save = {0, 0, 0, 0, 0, 0, 0, 0, false, false, false, false, false};

const str name = "Transistor Clicker";
const str version = "0.0.1_2";

const number cursorPrice = 15, mossPrice = 100, smallFABPrice = 1'000, mediumFABprice = 11'000, largeFABPrice = 120'000, intelI860Price = 1'305'078, startupPrice = 17'000'000, oakTreePrice = 200'000'000;
const number cursorYeild = 0.1, mossYeild = 1, smallFABYeild = 10, mediumFABYeild = 60, largeFABYeild = 260, intelI860Yeild = 1'700, startupYeild = 10'000, oakTreeYeild = 120'000;
const number expantionFactor = number(23, 20);

integer toInt(number x) {
	return integer(x);
}

str tolower(str s) {
	str ret = "";
	for(int i = 0; i < s.length(); i++) {
		ret += tolower(s[i]);
	}
	return ret;
}

number pow(number base, integer exponent) {
	number result = 1;
	for(integer i = 0; i < exponent; i++) {
		result *= base;
	}
	return result;
}

void addTrigger(Trigger trigger) {
	triggers.push_back(trigger);
}

void testTriggers(std::vector<str> args) {
	for(Trigger trigger : triggers) {
		if(trigger.condition(args)) {
			trigger.result(args);
		}
	}
}

number expandPrice(number price, integer count) {
	return price * pow(expantionFactor, count);
}

void printTitileCard() {
	std::cout << BOLDBLUE << name << ' ' << BOLDGREEN << version << RESET << "\n\n\n";
}

void clear(std::vector<str>&) {
	int bung = system("clear");
	printTitileCard();
}

str numString(number x, str thing, str plural = "s", bool round = true, str colA = BOLDGREEN, str colB = BOLDBLUE) {}

str TransitorsString(number transistors, bool round = true, str colA = BOLDGREEN, str colB = BOLDBLUE) {
	std::stringstream ss;
	if(round) transistors = GRP::round(transistors);
	str plural = GRP::round(transistors * 10) / 10 == 1 ? "transistor" : "transistors";
	ss << colA << GRP::toString(transistors) << ' ' << colB << plural;
	return ss.str();
}

void unlockCursor(std::vector<str>& args) {
	save.cursorUnlocked = true;
}

void unlockMoss(std::vector<str>& args) {
	save.mossUnlocked = true;
}

void unlockSmallFAB(std::vector<str>& args) {
	save.smallFABUnlocked = true;
}

void unlockMediumFAB(std::vector<str>& args) {
	save.mediumFABUnlocked = true;
}

void unlockLargeFAB(std::vector<str>& args) {
	save.largeFABUnlocked = true;
}

bool isCursorUnLocked(std::vector<str>& args) {
	return save.totalTransistors > cursorPrice;
}

bool isMossUnLocked(std::vector<str>& args) {
	return save.totalTransistors > mossPrice;
}

bool isSmallFABUnLocked(std::vector<str>& args) {
	return save.totalTransistors > smallFABPrice;
}

bool isMediumFABUnLocked(std::vector<str>& args) {
	return save.totalTransistors > mediumFABprice;
}

bool isLargeFABUnLocked(std::vector<str>& args) {
	return save.totalTransistors > largeFABPrice;
}

void increaseTransistors(number by) {
	save.transistorBalance += by;
	save.totalTransistors += by;
}

number calcCursorYeild() {
	return save.cusors * cursorYeild;
}

number calcMossYeild() {
	return save.moss * mossYeild;
}

number calcSmallFABYeild() {
	return save.smallFABs * smallFABYeild;
}

number calcMediumFABYeild() {
	return save.mediumFABs * mediumFABYeild;
}

number calcLargeFABYeild() {
	return save.largeFABs * largeFABYeild;
}

number calcTPS() {
	number TPS = 0;
	TPS += calcCursorYeild();
	TPS += calcMossYeild();
	TPS += calcSmallFABYeild();
	TPS += calcMediumFABYeild();
	TPS += calcLargeFABYeild();

	return TPS;
}

void onTick() {
	number TPS = calcTPS();
	increaseTransistors(TPS / 16);
	testTriggers(CMD::arguments);
}

void onExit(std::vector<str>&) {
	std::cout << BOLDRED << "Exiting Human Controll Console...\n";
	CMD::log("Terminating...");
}

void click() {
	number clickValue = 1;
	increaseTransistors(clickValue);
	save.clicks++;
	std::cout << BOLDWHITE << "Click! " << BOLDBLUE << "You made " << TransitorsString(clickValue) << RESET << '\n';
}

void balance(std::vector<str>& args) {
	CMD::log("Ran \"balance\" command");
	
	if(args.size() >= 2) {
		if(args[0] == "building") {
			integer count;
			number yeild;
			number producing;
			number TPS = calcTPS();
			number basePrice;
			str buildingName = args[1];
			bool unlocked;

			args[1] = tolower(args[1]);

			if(args[1] == "i860" || args[1] == "startup" || args[1] == "oaktree") {
				std::cout << "Coming soon!\n";
				return;
			}

			switch(hash(args[1])) {
				case cursor:
					buildingName = "cursor";
				    count = save.cusors;
				    producing = calcCursorYeild();
					basePrice = cursorPrice;
					unlocked = save.cursorUnlocked;
				    break;
				case moss:
				    buildingName = "moss";
				    count = save.moss;
				    producing = calcMossYeild();
					basePrice = mossPrice;
					unlocked = save.mossUnlocked;
				    break;
				case smallFAB:
				    buildingName = "small FAB";
				    count = save.smallFABs;
				    producing = calcSmallFABYeild();
					basePrice = smallFABPrice;
					unlocked = save.smallFABUnlocked;
				    break;
				case mediumFAB:
				    buildingName = "medium FAB";
				    count = save.mediumFABs;
				    producing = calcMediumFABYeild();
					basePrice = mediumFABprice;
					unlocked = save.mediumFABUnlocked;
				    break;
				case largeFAB:
				    buildingName = "large FAB";
				    count = save.largeFABs;
				    producing = calcLargeFABYeild();
					basePrice = largeFABPrice;
					unlocked = save.largeFABUnlocked;
				    break;
				default:
					unlocked = false;
				    break;
			}

			if(!unlocked) {
				std::cout << BOLDRED << "Unknown building!\n";
				return;
			}
			if(count > 0) {
			yeild = producing / count;
				
		    number percent = producing / TPS * 100;

			std::cout << BOLDBLUE << "You have " << BOLDGREEN << count << BOLDBLUE << ' ' << buildingName << "s, each producing " << TransitorsString(yeild) << " per second.\n";
			std::cout << "which produces " << TransitorsString(producing, false) << " per second in total which accounts for " << BOLDGREEN << GRP::toString(percent) << "%" << BOLDBLUE << " of your total TPS.\n";
			std::cout << "One " << buildingName << " would cost " << TransitorsString(expandPrice(basePrice, count)) << ".\n";
			return;
			} else {
				std::cout << BOLDBLUE << "You have no " << buildingName << "s.\n";
				std::cout << "One " << buildingName << " would cost " << TransitorsString(expandPrice(basePrice, count)) << ".\n";
				return;
			}
		}
	}
	std::cout << BOLDBLUE << "You have " << TransitorsString(save.transistorBalance) << ".\n";
	std::cout << BOLDBLUE << "You have made " << TransitorsString(save.totalTransistors) << " in total.\n";
	std::cout << BOLDBLUE << "You make " << TransitorsString(calcTPS(), false) << " per second.\n";

	CMD::log("They have" + TransitorsString(save.transistorBalance, "", "") + "!");
}

void buy(std::vector<str>& args) {
	if(args.size() < 1) {
		std::cout << BOLDRED << "You must specify somthing to buy!\n";
		return;
	}
	str building = tolower(args[0]);

	if(building == "i860" || building == "startup" || building == "oaktree") {
		std::cout << "coming soon!\n";
	}

	str buildingName;
	number basePrice;
	integer* countPtr = nullptr;
	bool buildingUnlocked = false;

	switch(hash(building)) {
		case cursor:
		    buildingName = "cursor";
		    basePrice = cursorPrice;
			countPtr = &save.cusors;
			buildingUnlocked = save.cursorUnlocked;
		    break;
		case moss:
		    buildingName = "moss";
		    basePrice = mossPrice;
		    countPtr = &save.moss;
			buildingUnlocked = save.mossUnlocked;
		    break;
		case smallFAB:
		    buildingName = "small FAB";
		    basePrice = smallFABPrice;
		    countPtr = &save.smallFABs;
			buildingUnlocked = save.smallFABUnlocked;
		    break;
		case mediumFAB:
		    buildingName = "medium FAB";
		    basePrice = mediumFABprice;
		    countPtr = &save.mediumFABs;
			buildingUnlocked = save.mediumFABUnlocked;
		    break;
		case largeFAB:
		    buildingName = "large FAB";
		    basePrice = largeFABPrice;
		    countPtr = &save.largeFABs;
			buildingUnlocked = save.largeFABUnlocked;
		    break;
		default:
		    buildingUnlocked = false;
			break;
	}

	if(!buildingUnlocked) {
		std::cout << BOLDRED << "Unknown building!\n";
		return;
	}
	integer& count = *countPtr;

	if(args.size() >= 2) {
		if(args[1] == "max") {
			integer totalBought = 0;
			number finalPrice = 0;

			while(save.transistorBalance >= finalPrice) {
				totalBought++;
				finalPrice += expandPrice(basePrice, count + totalBought);
			}

			totalBought--;
			finalPrice -= expandPrice(basePrice, count + totalBought);

			count += totalBought;
			save.transistorBalance -= finalPrice;

			if(count <= 0) {
				std::cout << BOLDRED << "You can't afford a " << buildingName << "!\n";
				return;
			}

			std::cout << BOLDBLUE << "You bought " << totalBought << " " << buildingName << "s for " << TransitorsString(finalPrice) << "!\n";

			return;
		}
		
		integer toPurchase(args[1]);
		number totalPrice = 0;

		for(integer i = 0; i < toPurchase; i++) {
			totalPrice += expandPrice(basePrice, i+count);
		}

		if(save.transistorBalance >= totalPrice) {
			save.transistorBalance -= totalPrice;
			count += toPurchase;

			std::cout << BOLDBLUE << "bought " << toPurchase << " " << buildingName << "s for " << TransitorsString(totalPrice) << "!\n";
		} else {
			std::cout << BOLDRED << "You don't have enough transistors to buy " << toPurchase << " for " << TransitorsString(totalPrice, BOLDRED, BOLDRED) << "!\n";
		}
	} else {
		if(save.transistorBalance >= expandPrice(basePrice, count)) {
			save.transistorBalance -= expandPrice(basePrice, count);
			count++;
		}

		std::cout << BOLDBLUE << "bought a " << buildingName << " for " << TransitorsString(expandPrice(basePrice, count - 1)) << "!\n";
	}
}

void help(std::vector<str>& args) {
	if(args.size() < 1) {
		std::cout << BOLDBLUE << "Commands:\n";
		std::cout << BOLDBLUE << "balance " << RESET << " - Shows info about your transistor balance\n";
		std::cout << BOLDBLUE << "buy " << RESET << " - Buys the selected item\n";
		std::cout << BOLDBLUE << "clear " << RESET << " - Clears the console\n";
		std::cout << BOLDBLUE << "help " << RESET << " - Gives assistance\n";
	} else if(args.size() >= 1) {
		if(args[0] == "balance") {
			if(args.size() == 1) {
			std::cout << BOLDWHITE << "BALANCE\n";
			std::cout << RESET << "Show numbers-related info about specified thing\n";
			std::cout << BOLDCYAN << "\nUsage:\n";
			std::cout << RESET << "balance [thing]\n";
			std::cout << BOLDCYAN << "\nThings:\n";
			std::cout << BOLDBLUE << "defualt" << RESET << " - transistor\n";
			std::cout << BOLDCYAN << "Subcommands:\n";
			std::cout << BOLDBLUE << "building" << RESET << " - Shows numbers-related info about a building\n";
			} else if(args.size() == 2) {
				if(args[1] == "building") {
					std::cout << BOLDWHITE << "BALANCE BUILDING\n";
					std::cout << RESET << "Show numbers-related info about a building\n";
					std::cout << BOLDCYAN << "\nUsage:\n";
					std::cout << RESET << "balance building [building]\n";
					std::cout << BOLDCYAN << "\nThings:\n";
					std::cout << BOLDBLUE << "cursor" << RESET << " - show cursor count, TPS and price for next cursor\n";
					std::cout << BOLDBLUE << "moss" << RESET << " - show moss count, TPS and price for next moss\n";
					std::cout << BOLDBLUE << "smallFAB" << RESET << " - show small FAB count, TPS and price for next small FAB\n";
					std::cout << BOLDBLUE << "mediumFAB" << RESET << " - show medium FAB count, TPS and price for next medium FAB\n";
					std::cout << BOLDBLUE << "largeFAB" << RESET << " - show large FAB count, TPS and price for next large FAB\n";
				}
			}
		} else if(args[0] == "buy") {
			{
			std::cout << BOLDWHITE << "BUY\n";
			std::cout << RESET << "Buy the specified item, if count is empty it will buy 1, if count is max it will buy the maximum ammount\n";
			std::cout << BOLDCYAN << "\nUsage:\n";
			std::cout << RESET << "buy [item] [count]\n";
			std::cout << BOLDCYAN << "\nThings:\n";
			std::cout << BOLDBLUE << "cursor" << RESET << " - buy a cursor\n";
			std::cout << BOLDBLUE << "moss" << RESET << " - buy some moss\n";
			std::cout << BOLDBLUE << "smallFAB" << RESET << " - buy a small FAB\n";
			std::cout << BOLDBLUE << "mediumFAB" << RESET << " - buy a medium FAB\n";
			std::cout << BOLDBLUE << "largeFAB" << RESET << " - buy a large FAB\n";
			}
		} else if(args[0] == "clear") {
			std::cout << BOLDWHITE << "CLEAR\n";
			std::cout << RESET << "Clears the console, then prints title card\n";
			std::cout << BOLDCYAN << "\nUsage:\n";
			std::cout << RESET << "clear\n";
		} else if(args[0] == "help") {
			std::cout << BOLDWHITE << "HELP\n";
			std::cout << RESET << "Shows help for commands, if none is specified then gives help on all commands\n";
			std::cout << BOLDCYAN << "\nUsage:\n";
			std::cout << RESET << "help [command]\n";
		}
	}
}

int main() {
	GRP::init();
	
	std::jthread gameThread = CMD::init(name, BOLDGREEN + str("@HCC") + BOLDBLUE + " ~/You" + RESET + "$ ", onTick);

	CMD::exit = onExit;

	/*str buildings[] = {"cursor", "moss", "smallFAB", "mediumFAB", "largeFAB", "i860", "startup", "oak tree"};
	for(int i = 0; i < 8; i++) {
		std::stringstream ss;
		ss << buildings[i] << ":" << hash(tolower(buildings[i]));
		CMD::log(ss.str());
	}*/

	CMD::addcommand("balance", balance);
	CMD::addcommand("buy", buy);
	CMD::addcommand("clear",  clear);
	CMD::addcommand("help", help);

	addTrigger({isCursorUnLocked, unlockCursor});
	addTrigger({isMossUnLocked, unlockMoss});
	addTrigger({isSmallFABUnLocked, unlockSmallFAB});
	addTrigger({isMediumFABUnLocked, unlockMediumFAB});
	addTrigger({isLargeFABUnLocked, unlockLargeFAB});

	CMD::log("Program iniitialized");

	CMD::runcomm("clear", click);

	CMD::command_loop(click);

	CMD::kill(gameThread);

	return 0;
}
