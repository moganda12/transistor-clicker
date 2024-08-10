#include <gmpxx.h>
#include <math.h>
#include <sstream>

#include "./lib/CMD.hpp"
#include "./lib/GRP.hpp"

#define RESET   "\033[0m"       /* Reset */
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

struct Upgrade {
    str name;
    str description;
    str effect;
    str flavortext;
    number cost;
    bool unlocked;
    bool purchased;
};

struct Acheivement {
    str name;
    str description;
    str flavortext;
    bool completed;
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
    std::vector<Upgrade> upgrades;
    std::vector<Acheivement> acheivements;
};

class UpgradeGroup {
private:
    std::vector<size_t> upgradeIndices;
    std::vector<Upgrade>* upgradeList;
public:
    UpgradeGroup(std::vector<Upgrade>* upgradeList) {
		this->upgradeList = upgradeList;
	}

    Upgrade& operator[] (size_t index) {
	    std::vector<Upgrade>& Ulist = *upgradeList;
		return Ulist[upgradeIndices[index]];
	}

    void append(size_t index) {
		upgradeIndices.push_back(index);
	}

    size_t size() {
		return upgradeIndices.size();
	}

class Iterator {
private:
    size_t index;
    UpgradeGroup& group;

public:
    Iterator(size_t index, UpgradeGroup& group) : index(index), group(group) {}

    void operator++() {
		index++;
	}

    Upgrade& operator*() {
		return this->group[this->index];
	}

    bool operator!=(const Iterator& other) {
		return this->index != other.index;
	}
};

    Iterator begin() {
		return Iterator(0, *this);
	}

    Iterator end() {
		return Iterator(this->upgradeIndices.size(), *this);
	}
};


str tolower(str s) {
	str ret = "";
	for(int i = 0; i < s.length(); i++) {
		ret += tolower(s[i]);
	}
	return ret;
}

struct Trigger {
    CMD::Condition condition;
    CMD::Result result;
    bool selfdelete = true;
};

std::vector<Trigger> triggers;

std::unordered_map<str, size_t> upgradeIndex;
std::unordered_map<str, size_t> acheivementIndex;

TclickerSave save = {0, 0, 0, 0, 0, 0, 0, 0, false, false, false, false, false, {}, {}};

const str name = "Transistor Clicker";
const str version = "0.0.1_2";

const number cursorPrice = 15, mossPrice = 100, smallFABPrice = 1'000, mediumFABprice = 11'000, largeFABPrice = 120'000, intelI860Price = 1'305'078, startupPrice = 17'000'000, oakTreePrice = 200'000'000;
const number cursorYeild = 0.1, mossYeild = 1, smallFABYeild = 10, mediumFABYeild = 60, largeFABYeild = 260, intelI860Yeild = 1'700, startupYeild = 10'000, oakTreeYeild = 120'000;
const number expantionFactor = number(23, 20);

UpgradeGroup cursorUpgrades(&save.upgrades);
UpgradeGroup mossUpgrades(&save.upgrades);
UpgradeGroup smallFABUpgrades(&save.upgrades);
UpgradeGroup mediumFABUpgrades(&save.upgrades);
UpgradeGroup largeFABUpgrades(&save.upgrades);

void createUpgrade(str Uname, str description, str effect, str flavortext, number cost) {
	save.upgrades.push_back({Uname, description, effect, flavortext, cost, false, false});
	upgradeIndex[Uname] = save.upgrades.size() - 1;
}

void createUpgrade(str Uname, str description, str effect, str flavortext, number cost, UpgradeGroup& group) {
	save.upgrades.push_back({Uname, description, effect, flavortext, cost, false, false});
	upgradeIndex[Uname] = save.upgrades.size() - 1;
	group.append(upgradeIndex[Uname]);
}

void createAcheivement(str achName, str description, str flavortext) {
	save.acheivements.push_back({achName, description, flavortext, false});
	acheivementIndex[achName] = save.acheivements.size() - 1;
}

size_t getUpgradeByName(str Uname) {
	return upgradeIndex[tolower(Uname)];
}

size_t getAcheivementByHandle(str handle) {
	return acheivementIndex[handle];
}

integer toInt(number x) {
	return integer(x);
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

str numString(number x, str thing, str plural = "s", integer precision = 0, str colA = BOLDGREEN, str colB = BOLDBLUE) {
	std::stringstream result;

	str numStr = GRP::toString(x, precision);

	thing.append(numStr == "1" ? "" : plural);

	result << colA << numStr << ' ' << colB << thing;

	return result.str();
}

str TransitorsString(number transistors, integer precision = 0, str colA = BOLDGREEN, str colB = BOLDBLUE) {
	return numString(transistors, "transistor", "s", precision, colA, colB);
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

bool canUnLockIntegratedMouse(std::vector<str>& args) {
	return save.cusors >= 1;
}

bool canUnLockFastFing(std::vector<str>& args) {
	return save.cusors >= 1;
}

bool canUnLockChippy(std::vector<str>& args) {
	return save.cusors >= 10;
}

bool canUnLockMossyMossy(std::vector<str>& args) {
	return save.moss >= 1;
}

bool canUnLockMossWalls(std::vector<str>& args) {
	return save.moss >= 5;
}

bool canUnLockCheapMachines(std::vector<str>& args) {
	return save.smallFABs >= 1;
}

bool canUnLockDenseChips(std::vector<str>& args) {
	return save.smallFABs >= 5;
}

bool canUnLockMossyTech(std::vector<str>& args) {
	return save.mediumFABs >= 1;
}

bool canUnLockEndgame(std::vector<str>& args) {
	return save.largeFABs >= 1;
}

void unLockIntegratedMouse(std::vector<str>& args) {
	Upgrade& FasterFingers = save.upgrades[getUpgradeByName("integrated mouse")];
	FasterFingers.unlocked = true;
}

void unLockFastFing(std::vector<str>& args) {
	Upgrade& FasterFingers = save.upgrades[getUpgradeByName("faster fingers")];
	FasterFingers.unlocked = true;
}

void unLockChippy(std::vector<str>& args) {
	Upgrade& Chippy = save.upgrades[getUpgradeByName("chippy")];
	Chippy.unlocked = true;
}

void unLockMossyMossy(std::vector<str>& args) {
	Upgrade& MossyMossy = save.upgrades[getUpgradeByName("mossy mossy")];
	MossyMossy.unlocked = true;
}

void unLockMossWalls(std::vector<str>& args) {
	Upgrade& MossyMossy = save.upgrades[getUpgradeByName("moss walls")];
	MossyMossy.unlocked = true;
}

void unLockCheapMachines(std::vector<str>& args) {
	Upgrade& CheapMachines = save.upgrades[getUpgradeByName("cheap lithography machines")];
	CheapMachines.unlocked = true;
}

void unLockDenseChips(std::vector<str>& args) {
	Upgrade& DenseChips = save.upgrades[getUpgradeByName("denser chips")];
	DenseChips.unlocked = true;
}

void unLockMossyTech(std::vector<str>& args) {
	Upgrade& MossyTech = save.upgrades[getUpgradeByName("mossier tech")];
	MossyTech.unlocked = true;
}

void unLockEndgame(std::vector<str>& args) {
	Upgrade& Endgame = save.upgrades[getUpgradeByName("endgame")];
	Endgame.unlocked = true;
}

number calcCursorYeild() {
	number yeild = cursorYeild;

	for(Upgrade& upgrade : cursorUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	return yeild * save.cusors;
}

number calcMossYeild() {
	number yeild = mossYeild;

	for(Upgrade& upgrade : mossUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	return yeild * save.moss;
}

number calcSmallFABYeild() {
	number yeild = smallFABYeild;

	for(Upgrade& upgrade : smallFABUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	for(Upgrade& upgrade : mediumFABUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	for(Upgrade& upgrade : largeFABUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	return yeild *= save.smallFABs;
}

number calcMediumFABYeild() {
	number yeild = mediumFABYeild;

	for(Upgrade& upgrade : mediumFABUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	for(Upgrade& upgrade : largeFABUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	return yeild * save.mediumFABs;
}

number calcLargeFABYeild() {
	number yeild = largeFABYeild;

	for(Upgrade& upgrade : largeFABUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	return yeild * save.largeFABs;
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

number calcClickYeild() {
	number yeild = 1;

	for(size_t i = 0; i < cursorUpgrades.size(); i++) {
		if(cursorUpgrades[i].purchased) yeild *= 2;
	}

	return yeild;
}

void click() {
	number clickValue = calcClickYeild();
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

			std::cout << BOLDBLUE << "You have " << numString(count, buildingName) <<", each producing " << TransitorsString(yeild, 1) << " per second.\n";
			std::cout << "which produces " << TransitorsString(producing, 1) << " per second in total which accounts for " << BOLDGREEN << GRP::toString(percent, 2) << "%" << BOLDBLUE << " of your total TPS.\n";
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
	std::cout << BOLDBLUE << "You make " << TransitorsString(calcTPS(), 1) << " per second.\n";

	CMD::log("They have " + TransitorsString(save.transistorBalance, 0, "", "") + "!");
}

void buy(std::vector<str>& args) {
	if(args.size() > 1) {
		if(args[0] == "upgrade") {
			str Uname = "";

			for(size_t i = 1; i < args.size(); i++) {
				Uname += args[i];
				if(i != args.size() - 1) Uname += " ";
			}

			if(upgradeIndex.find(tolower(Uname)) == upgradeIndex.end()) {
				std::cout << BOLDRED << "Unknown upgrade!\n";
				return;
			}
			
			Upgrade& upgrade = save.upgrades[getUpgradeByName(Uname)];

			if(!upgrade.unlocked) {
				std::cout << BOLDRED << "Unkown upgrade!\n";
				return;
			}

			if(upgrade.purchased) {
				std::cout << BOLDRED << "You already have this upgrade!\n";
				return;
			}

			if(save.transistorBalance >= upgrade.cost) {
				save.transistorBalance -= upgrade.cost;
				upgrade.purchased = true;

				std::cout << BOLDBLUE << "You bought " << upgrade.name << " for " << TransitorsString(upgrade.cost) << ".\n";
				return;
			} else {
				std::cout << BOLDRED << "You don't have enough transistors to buy " << upgrade.name;
				return;
			}
		}
	}
	
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

			std::cout << BOLDBLUE << "You bought " << numString(totalBought, buildingName) << " for " << TransitorsString(finalPrice) << "!\n";

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

			std::cout << BOLDBLUE << "bought " << numString(toPurchase, buildingName) << " for " << TransitorsString(totalPrice) << "!\n";
		} else {
			std::cout << BOLDRED << "You don't have enough transistors to buy " << toPurchase << " for " << TransitorsString(totalPrice, 0, BOLDRED, BOLDRED) << "!\n";
		}
	} else {
		if(save.transistorBalance >= expandPrice(basePrice, count)) {
			save.transistorBalance -= expandPrice(basePrice, count);
			count++;
		} else {
			std::cout << BOLDRED << "You don't have enough transistors to buy a " << buildingName << "!\n";
			return;
		}

		std::cout << BOLDBLUE << "bought a " << buildingName << " for " << TransitorsString(expandPrice(basePrice, count - 1)) << "!\n";
	}
}

void info(std::vector<str>& args) {
	if(args.size() < 1) {
		std::cout << BOLDRED << "You must specify somthing to be informed about!\n";
		return;
	} else if(args.size() == 1) {
		if(args[0] == "transistors" || args[0] == "transistor") {
			std::cout << BOLDWHITE << "TRANSISTORS\n";
			std::cout << BOLDCYAN << "Description:\n";
			std::cout << RESET << "Transistors are the most common semiconductor, they\n";
			std::cout << "are switched by electricity, because of this they are used to make\n";
			std::cout << "most computer parts, they are also the main currency of the game!\n";
			std::cout << BOLDCYAN << "Uses:\n";
			std::cout << RESET << "Transistors, being the main currency, are used to buy\n";
			std::cout << "most things in-game!\n\n";
			std::cout << BOLDWHITE << "\"Moorgans law: the ammount of transistors in existance will double every hour\"\n";
		}
	} else if(args.size() > 1) {
		if(args[0] == "building") {
			args[1] = tolower(args[1]);
			if(args[1] == "i860" || args[1] == "startup" || args[1] == "oaktree") {
				std::cout << "coming soon!\n";
				return;
			}

			if(args[1] == "cursor") {
				std::cout << BOLDWHITE << "CURSOR\n";
				std::cout << BOLDCYAN << "Description:\n";
				std::cout << RESET << "Autoclicks once every 10 seconds\n";
				std::cout << BOLDCYAN << "Effect:\n";
				std::cout << RESET << "Makes " << TransitorsString(cursorYeild) << " every ten seconds\n\n";
				std::cout << BOLDWHITE << "\"A nice cusor for your nice mouse\"\n";
			} else if(args[1] == "moss") {}
			
		} else if(args[0] == "upgrade") {
			str Uname = "";

			for(size_t i = 1; i < args.size(); i++) {
				Uname += args[i];
				if(i != args.size() - 1) Uname += " ";
			}

			if(upgradeIndex.find(tolower(Uname)) == upgradeIndex.end()) {
				std::cout << BOLDRED << "Unknown upgrade!\n";
				return;
			}

			Upgrade& upgrade = save.upgrades[getUpgradeByName(Uname)];

			if(!upgrade.unlocked) {
				std::cout << BOLDRED << "Unkown upgrade!\n";
				return;
			}
			
			std::cout << BOLDWHITE << "UPGRADE: " << upgrade.name << '\n';
			std::cout << BOLDCYAN << "Description:\n";
			std::cout << RESET << upgrade.description << '\n';
			std::cout << BOLDCYAN << "Effect:\n";
			std::cout << RESET << upgrade.effect << "\n\n";
			std::cout << BOLDWHITE << '"' << upgrade.flavortext << "\"\n";
		}
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
		} else if(args[0] == "info") {
			std::cout << BOLDWHITE << "INFO\n";
			std::cout << RESET << "Shows qualatative information about things in the game\n";
			std::cout << BOLDCYAN << "\nUsage:\n";
			std::cout << RESET << "info [thing]\n";
			std::cout << BOLDCYAN << "\nThings:\n";
			std::cout << RESET << "Inummerable\n";
			std::cout << BOLDCYAN << "Subcommands:\n";
			std::cout << BOLDBLUE << "buildings" << RESET << " - Shows info about selected building\n";
			std::cout << BOLDBLUE << "upgrades" << RESET << " - Shows info about selected upgrade\n";
		}
	}
}

void list(std::vector<str>& args) {
	args.swap(args);

	for(Upgrade& upgrade : save.upgrades) {
		if(upgrade.unlocked) {
			std::cout << BOLDBLUE << upgrade.name << RESET << " - " << upgrade.description << "\n";
			std::cout << "it will cost " << TransitorsString(upgrade.cost) << "\n\n";
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
	CMD::addcommand("info", info);
	CMD::addcommand("list", list);

	createUpgrade("integrated mouse", "the mouse now integrates semiconductor technology into it's design", "doubles mouse and cursor output.", "Now with semiconductor technology!", 100, cursorUpgrades);
	createUpgrade("faster fingers", "makes fingers faster", "doubles mouse and cursor output.", "Buy our finger speed pills today, double finger speed garauteed!", 500, cursorUpgrades);
	createUpgrade("chippy", "an assistant to improve your clicking methods.", "doubles mouse and cursor output.", "do you want to click a button? how about writing a letter instead.", 10'000, cursorUpgrades);

	addTrigger({canUnLockIntegratedMouse, unLockIntegratedMouse});
	addTrigger({canUnLockFastFing, unLockFastFing});
	addTrigger({canUnLockChippy, unLockChippy});

	createUpgrade("mossy mossy", "Boosts moss production... somehow.", "doubles moss output.", "Mossy Mossy", 1'000, mossUpgrades);
	createUpgrade("moss walls", "Add moss walls to the office space", "doubles moss output.", "Add moss walls to the halls so the moss has more space to grow.", 5'000, mossUpgrades);
	
	addTrigger({canUnLockMossyMossy, unLockMossyMossy});
	addTrigger({canUnLockMossWalls, unLockMossWalls});

	createUpgrade("cheap lithography machines", "makes small FABs better!", "doubles small FAB output", "Cheaper lithography machines make small FABs more cost effective.", 10'000, smallFABUpgrades);
	createUpgrade("denser chips", "more transistors fit on the same chip!", "doubles small FAB output.", "All new chipmaking process doubles chip density and performance!", 50'000, smallFABUpgrades);

	addTrigger({canUnLockCheapMachines, unLockCheapMachines});
	addTrigger({canUnLockDenseChips, unLockDenseChips});

	createUpgrade("mossier tech", "includes moss in microchips.", "doubles small & meduim FAB output", "Whitness the pure power of all new MOSS transistors", 110'000, mediumFABUpgrades);

	addTrigger({canUnLockMossyTech, unLockMossyTech});

	createUpgrade("endgame", "the game is complete...\n\nfor now", "doubles all FABs output", "huh I thought there'd be more", 200'000, largeFABUpgrades);

	addTrigger({canUnLockEndgame, unLockEndgame});

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
