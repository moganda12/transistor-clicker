#include <gmpxx.h>
#include <math.h>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "./lib/CMD.hpp"
#include "./lib/GRP.hpp"
#include "./lib/json.hpp"

#pragma region definitions

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

#define DEBUG

using str = std::string;
using json = nlohmann::json;

using integer = mpz_class;
using flat = mpf_class;
using number = mpq_class;

std::hash<str> hasher;

long hash(str s) {
	return hasher(s);
}

#pragma endregion





#pragma region structs & classes

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

struct Trigger {
    CMD::Condition condition;
    CMD::Result result;
    bool selfdelete = true;
};

struct Upgrade {
    str name;
    str description;
    str effect;
    str flavortext;
    number cost;
    bool unlocked;
    bool purchased;
	Trigger unlockTrigger;
};

struct Achievement {
    str name;
    str description;
    str flavortext;
    bool earned;
	Trigger unlockTrigger;
};

struct TclickerState {
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

struct Notification {
	str title;
	str content;
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

	void clear() {
		upgradeIndices.clear();
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

#pragma endregion





#pragma region variables

std::vector<Trigger> triggers;

std::vector<Upgrade> upgrades;
std::vector<Achievement> achievements;
std::vector<Notification> notifications;

std::unordered_map<str, size_t> upgradeIndex;
std::unordered_map<str, size_t> achievementIndex;

TclickerState gameState = {0, 0, 0, 0, 0, 0, 0, 0, false, false, false, false, false};

const str name = "Transistor Clicker";
const str version = "0.0.2_2";

const number cursorPrice = 15, mossPrice = 100, smallFABPrice = 1'000, mediumFABPrice = 11'000, largeFABPrice = 120'000, intelI860Price = 1'305'078, startupPrice = 17'000'000, oakTreePrice = 200'000'000;
const number cursorYeild = number(1, 10), mossYeild = 1, smallFABYeild = 10, mediumFABYeild = 60, largeFABYeild = 260, intelI860Yeild = 1'700, startupYeild = 10'000, oakTreeYeild = 120'000;
const number expantionFactor = number(23, 20);
number tPSCache = 0;

UpgradeGroup cursorUpgrades(&upgrades);
UpgradeGroup mossUpgrades(&upgrades);
UpgradeGroup smallFABUpgrades(&upgrades);
UpgradeGroup mediumFABUpgrades(&upgrades);
UpgradeGroup largeFABUpgrades(&upgrades);

str gameName;

#pragma endregion





#pragma region number stuff

integer getrationalnumerator(number n) {
	return integer(n.get_num_mpz_t());
}

integer getrationaldenominator(number n) {
	return integer(n.get_den_mpz_t());
}

integer toInt(number x) {
	return integer(x);
}

number pow(number base, integer exponent) {
	number result = 1;
	for(integer i = 0; i < exponent; i++) {
		result *= base;
	}
	result.canonicalize();
	return result;
}

str numString(number x, str thing, str plural = "s", integer precision = 0, str colA = BOLDGREEN, str colB = BOLDBLUE) {
	std::stringstream result;

	str numStr = GRP::toString(x, precision);

	thing.append(numStr == "1" ? "" : plural);

	result << colA << numStr << ' ' << colB << thing;

	return result.str();
}

number expandPrice(number price, integer count) {
	return price * pow(expantionFactor, count);
}

str TransitorsString(number transistors, integer precision = 0, str colA = BOLDGREEN, str colB = BOLDBLUE) {
	return numString(transistors, "transistor", "s", precision, colA, colB);
}

#pragma endregion





#pragma region misc

void increaseTransistors(number by) {
	gameState.transistorBalance += by;
	gameState.totalTransistors += by;
}

str tolower(str s) {
	str ret = "";
	for(int i = 0; i < s.length(); i++) {
		ret += tolower(s[i]);
	}
	return ret;
}

void printTitileCard() {
	std::cout << BOLDBLUE << name << ' ' << BOLDGREEN << version << RESET << '\n';
	std::cout << YELLOW << "Save & Load Your Upgrades & Achievements\n\n\n" << RESET;
}

#pragma endregion





#pragma region yeild calc

number calcCursorYeild() {
	number yeild = cursorYeild;

	for(Upgrade& upgrade : cursorUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	return yeild;
}

number calcMossYeild() {
	number yeild = mossYeild;

	for(Upgrade& upgrade : mossUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	return yeild;
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

	return yeild;
}

number calcMediumFABYeild() {
	number yeild = mediumFABYeild;

	for(Upgrade& upgrade : mediumFABUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	for(Upgrade& upgrade : largeFABUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	return yeild;
}

number calcLargeFABYeild() {
	number yeild = largeFABYeild;

	for(Upgrade& upgrade : largeFABUpgrades) {
		if(upgrade.purchased) yeild *= 2;
	}

	return yeild;
}

number calcTPS() {
	number TPS = 0;
	TPS += calcCursorYeild() * gameState.cusors;
	TPS += calcMossYeild() * gameState.moss;
	TPS += calcSmallFABYeild() * gameState.smallFABs;
	TPS += calcMediumFABYeild() * gameState.mediumFABs;
	TPS += calcLargeFABYeild() * gameState.largeFABs;

	return TPS;
}

#pragma endregion





#pragma region engine stuff

void addTrigger(Trigger trigger) {
	triggers.push_back(trigger);
}

void testTriggers(std::vector<str> args) {
	for(int i = 0; i < triggers.size(); i++) {
		Trigger trigger = triggers[i];
		
		if(trigger.condition(args)) {
			trigger.result(args);

			if(trigger.selfdelete) {
				triggers.erase(triggers.begin() + i);
			}
		}
	}
}

void onTick() {
	number TPS = calcTPS();
	tPSCache = TPS;
	increaseTransistors(TPS / 16);
	testTriggers(CMD::arguments);
}

void onExit(std::vector<str>&) {
	std::cout << BOLDRED << "Exiting Human Controll Console...\n";
	CMD::log("Terminating...");
}

#pragma endregion





#pragma region upgrade & achievment schtuff

void createUpgrade(str Uname, str description, str effect, str flavortext, number cost, Trigger unlockTrigger) {
	upgrades.push_back({Uname, description, effect, flavortext, cost, false, false, unlockTrigger});
	upgradeIndex[tolower(Uname)] = upgrades.size() - 1;
}

void createUpgrade(str Uname, str description, str effect, str flavortext, number cost, UpgradeGroup& group, Trigger unlockTrigger) {
	upgrades.push_back({Uname, description, effect, flavortext, cost, false, false, unlockTrigger});
	upgradeIndex[tolower(Uname)] = upgrades.size() - 1;
	group.append(upgradeIndex[Uname]);
}

void createAchievement(str achName, str description, str flavortext, Trigger unlockTrigger) {
	achievements.push_back({achName, description, flavortext, false, unlockTrigger});
	achievementIndex[tolower(achName)] = achievements.size() - 1;
}

size_t getUpgradeByName(str Uname) {
	return upgradeIndex[tolower(Uname)];
}

size_t getAchievementByName(str Aname) {
	return achievementIndex[tolower(Aname)];
}

void addUpgradeTriggers() {
	for(Upgrade& upgrade : upgrades) {
		if(!upgrade.unlocked) {
			addTrigger(upgrade.unlockTrigger);
		}
	}
}

void addAchievementTriggers() {
	for(Achievement& achievement : achievements) {
		if(!achievement.earned) {
			addTrigger(achievement.unlockTrigger);
		}
	}
}

void unlockAchievement(str name) {
	achievements[getAchievementByName(name)].earned = true;
	notifications.push_back({"Achievement unlocked!", achievements[getAchievementByName(name)].name});
}

#pragma endregion





#pragma region THE WALL





bool alwayTrue(std::vector<str>& args) {
	return true;
}

void duZNutin(std::vector<str>& args) {
	//it really doez
}






#pragma region for upgrades

void unlockCursor(std::vector<str>& args) {
	gameState.cursorUnlocked = true;
}

void unlockMoss(std::vector<str>& args) {
	gameState.mossUnlocked = true;
}

void unlockSmallFAB(std::vector<str>& args) {
    gameState.smallFABUnlocked = true;
}

void unlockMediumFAB(std::vector<str>& args) {
	gameState.mediumFABUnlocked = true;
}

void unlockLargeFAB(std::vector<str>& args) {
	gameState.largeFABUnlocked = true;
}

bool isCursorUnLocked(std::vector<str>& args) {
	return gameState.totalTransistors > cursorPrice;
}

bool isMossUnLocked(std::vector<str>& args) {
	return gameState.totalTransistors > mossPrice;
}

bool isSmallFABUnLocked(std::vector<str>& args) {
	return gameState.totalTransistors > smallFABPrice;
}

bool isMediumFABUnLocked(std::vector<str>& args) {
	return gameState.totalTransistors > mediumFABPrice;
}

bool isLargeFABUnLocked(std::vector<str>& args) {
	return gameState.totalTransistors > largeFABPrice;
}

bool canUnLockIntegratedMouse(std::vector<str>& args) {
	return gameState.cusors >= 1;
}

bool canUnLockFastFing(std::vector<str>& args) {
	return gameState.cusors >= 1;
}

bool canUnLockChippy(std::vector<str>& args) {
	return gameState.cusors >= 10;
}

bool canUnLockMossyMossy(std::vector<str>& args) {
	return gameState.moss >= 1;
}

bool canUnLockMossWalls(std::vector<str>& args) {
	return gameState.moss >= 5;
}

bool canUnLockCheapMachines(std::vector<str>& args) {
	return gameState.smallFABs >= 1;
}

bool canUnLockDenseChips(std::vector<str>& args) {
	return gameState.smallFABs >= 5;
}

bool canUnLockMossyTech(std::vector<str>& args) {
	return gameState.mediumFABs >= 1;
}

bool canUnLockEndgame(std::vector<str>& args) {
	return gameState.largeFABs >= 1;
}

void unLockIntegratedMouse(std::vector<str>& args) {
	Upgrade& FasterFingers = upgrades[getUpgradeByName("integrated mouse")];
	FasterFingers.unlocked = true;
}

void unLockFastFing(std::vector<str>& args) {
	Upgrade& FasterFingers = upgrades[getUpgradeByName("faster fingers")];
	FasterFingers.unlocked = true;
}

void unLockChippy(std::vector<str>& args) {
	Upgrade& Chippy = upgrades[getUpgradeByName("chippy")];
	Chippy.unlocked = true;
}

void unLockMossyMossy(std::vector<str>& args) {
	Upgrade& MossyMossy = upgrades[getUpgradeByName("mossy mossy")];
	MossyMossy.unlocked = true;
}

void unLockMossWalls(std::vector<str>& args) {
	Upgrade& MossyMossy = upgrades[getUpgradeByName("moss walls")];
	MossyMossy.unlocked = true;
}

void unLockCheapMachines(std::vector<str>& args) {
	Upgrade& CheapMachines = upgrades[getUpgradeByName("cheap lithography machines")];
	CheapMachines.unlocked = true;
}

void unLockDenseChips(std::vector<str>& args) {
	Upgrade& DenseChips = upgrades[getUpgradeByName("denser chips")];
	DenseChips.unlocked = true;
}

void unLockMossyTech(std::vector<str>& args) {
	Upgrade& MossyTech = upgrades[getUpgradeByName("mossier tech")];
	MossyTech.unlocked = true;
}

void unLockEndgame(std::vector<str>& args) {
	Upgrade& Endgame = upgrades[getUpgradeByName("endgame")];
	Endgame.unlocked = true;
}

#pragma endregion





#pragma region for achievements

bool canUnLockClick(std::vector<str>& args) {
	return gameState.totalTransistors >= 1;
}

bool canUnLockSemigood(std::vector<str>& args) {
	return gameState.totalTransistors >= 1'000;
}

bool canUnLockTransistant(std::vector<str>& args) {
	return gameState.totalTransistors >= 100'000;
}

bool canUnLockCasualSemi(std::vector<str>& args) {
	return tPSCache >= 1;
}

bool canUnLockHarcoreSemi(std::vector<str>& args) {
	return tPSCache >= 10;
}

bool canUnLockSteadyStream(std::vector<str>& args) {
	return tPSCache >= 100;
}

bool canUnLockAutoClick(std::vector<str>& args) {
	return gameState.cusors >= 1;
}

bool canUnLockDoubleClick(std::vector<str>& args) {
	return gameState.cusors >= 2;
}

bool canUnLockMouseWheel(std::vector<str>& args) {
	return gameState.cusors >= 50;
}

bool canUnLockMossy(std::vector<str>& args) {
	return gameState.moss >= 1;
}

bool canUnLockMossGardens(std::vector<str>& args) {
	return gameState.moss >= 50;
}

bool canUnLockFABrication(std::vector<str>& args) {
	return gameState.smallFABs >= 1;
}

bool canUnLockDatamining(std::vector<str>& args) {
	return gameState.mediumFABs >= 1;
}

bool canUnLockThatsBig(std::vector<str>& args) {
	return gameState.largeFABs >= 1;
}

bool canUnLockFinal(std::vector<str>& args) {
	return upgrades[getUpgradeByName("endgame")].purchased;
}

void unLockClick(std::vector<str>& args) {
	unlockAchievement("click");
}

void unLockSemigood(std::vector<str>& args) {
	unlockAchievement("semigood");
}

void unLockTransistant(std::vector<str>& args) {
	unlockAchievement("transistant");
}

void unLockCasualSemi(std::vector<str>& args) {
	unlockAchievement("casual semiconductors");
}

void unLockHarcoreSemi(std::vector<str>& args) {
	unlockAchievement("hardcore semiconductors");
}

void unLockSteadyStream(std::vector<str>& args) {
	unlockAchievement("steady semistream");
}

void unLockAutoClick(std::vector<str>& args) {
	unlockAchievement("autoclick");
}

void unLockDoubleClick(std::vector<str>& args) {
	unlockAchievement("double click");
}

void unLockMouseWheel(std::vector<str>& args) {
	unlockAchievement("mouse wheel");
}

void unLockMossy(std::vector<str>& args) {
	unlockAchievement("mossy");
}

void unLockMossGardens(std::vector<str>& args) {
	unlockAchievement("moss gardens");
}

void unLockFABrication(std::vector<str>& args) {
	unlockAchievement("fabrication");
}

void unLockDatamining(std::vector<str>& args) {
	unlockAchievement("datamining");
}

void unLockThatsBig(std::vector<str>& args) {
	unlockAchievement("that's big");
}

void unLockFinal(std::vector<str>& args) {
	unlockAchievement("???");
}

#pragma endregion


#pragma endregion





#pragma region saves

integer json_read_integer_safe(json::value_type j, integer def = 0) {
	if(j.is_null()) {
		return def;
	}

	return integer((str)j);
}

bool json_bool_nullcheck(json::value_type j, bool def = false) {
	if(j.is_null()) {
		return def;
	}

	return j;
}

number json_read_number(json j, number def = 0) {
	if(j.find("numerator") == j.end() || j.find("denominator") == j.end()) {
		return def;
	}
	
	integer numerator = integer((str)j["numerator"]);
	integer denominator = integer((str)j["denominator"]);

	return number(numerator, denominator);
}

void json_dump_number(json& j, number n) {
	j["numerator"] = getrationalnumerator(n).get_str();
	j["denominator"] = getrationaldenominator(n).get_str();
}

void saveGame(str fname) {
	json saveData = json::parse("{}");

	gameState.transistorBalance.canonicalize();

	json_dump_number(saveData["transistorBalance"], gameState.transistorBalance);

	json_dump_number(saveData["totalTransistors"], gameState.totalTransistors);

	json& buildings = saveData["buildings"];

	json& cursor = buildings["cursor"];
	json& moss = buildings["moss"];
	json& smallFAB = buildings["smallFAB"];
	json& mediumFAB = buildings["mediumFAB"];
	json& largeFAB = buildings["largeFAB"];
	
	cursor["count"] = gameState.cusors.get_str();
	moss["count"] = gameState.moss.get_str();
	smallFAB["count"] = gameState.smallFABs.get_str();
	mediumFAB["count"] = gameState.mediumFABs.get_str();
	largeFAB["count"] = gameState.largeFABs.get_str();

	json& upgradesJson = saveData["upgrades"];

	for(Upgrade& upgrade : upgrades) {
		json& upgradeJson = upgradesJson[upgrade.name];
		upgradeJson["bought"] = upgrade.purchased;
		upgradeJson["unlocked"] = upgrade.unlocked;
	}

	json& achievementsJson = saveData["achievements"];

	for(Achievement& achievement : achievements) {
		json& achievementJson = achievementsJson[achievement.name];
		achievementJson["earned"] = achievement.earned;
	}

	std::remove(fname.c_str());

	std::ofstream saveFile(fname);
	saveFile << saveData.dump(4) << std::flush;
}

void loadGame(str fname) {
	std::ifstream saveFile(fname);
	json saveData = json::parse(saveFile);

	gameState.transistorBalance = json_read_number(saveData["transistorBalance"]);
	gameState.totalTransistors = json_read_number(saveData["totalTransistors"]);

	json& buildings = saveData["buildings"];

	json& cursor = buildings["cursor"];
	json& moss = buildings["moss"];
	json& smallFAB = buildings["smallFAB"];
	json& mediumFAB = buildings["mediumFAB"];
	json& largeFAB = buildings["largeFAB"];

	gameState.cusors = json_read_integer_safe(cursor["count"]);
	gameState.moss = json_read_integer_safe(moss["count"]);
	gameState.smallFABs = json_read_integer_safe(smallFAB["count"]);
	gameState.mediumFABs = json_read_integer_safe(mediumFAB["count"]);
	gameState.largeFABs = json_read_integer_safe(largeFAB["count"]);

	json& upgradesJson = saveData["upgrades"];

	for(Upgrade& upgrade : upgrades) {
		json& upgradeJson = upgradesJson[upgrade.name];
		upgrade.purchased = json_bool_nullcheck(upgradeJson["bought"]);
		upgrade.unlocked  = json_bool_nullcheck(upgradeJson["unlocked"]);
	}

	json& achievementsJson = saveData["achievements"];

	for(Achievement& achievement : achievements) {
		json& achievementJson = achievementsJson[achievement.name];
		achievement.earned = json_bool_nullcheck(achievementJson["earned"]);
	}
}

#pragma endregion


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
	gameState.clicks++;
	std::cout << BOLDWHITE << "Click! " << BOLDBLUE << "You made " << TransitorsString(clickValue) << RESET << '\n';
}

#pragma region commands

void clear(std::vector<str>& args)   {
	int bung = system("clear");
	printTitileCard();
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
				    count = gameState.cusors;
				    yeild = calcCursorYeild();
					basePrice = cursorPrice;
					unlocked = gameState.cursorUnlocked;
				    break;
				case moss:
				    buildingName = "moss";
				    count = gameState.moss;
				    yeild = calcMossYeild();
					basePrice = mossPrice;
					unlocked = gameState.mossUnlocked;
				    break;
				case smallFAB:
				    buildingName = "small FAB";
				    count = gameState.smallFABs;
				    yeild = calcSmallFABYeild();
					basePrice = smallFABPrice;
					unlocked = gameState.smallFABUnlocked;
				    break;
				case mediumFAB:
				    buildingName = "medium FAB";
				    count = gameState.mediumFABs;
				    yeild = calcMediumFABYeild();
					basePrice = mediumFABPrice;
					unlocked = gameState.mediumFABUnlocked;
				    break;
				case largeFAB:
				    buildingName = "large FAB";
				    count = gameState.largeFABs;
				    yeild = calcLargeFABYeild();
					basePrice = largeFABPrice;
					unlocked = gameState.largeFABUnlocked;
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
			producing = yeild * count;
				
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
	std::cout << BOLDBLUE << "You have " << TransitorsString(gameState.transistorBalance) << ".\n";
	std::cout << BOLDBLUE << "You have made " << TransitorsString(gameState.totalTransistors) << " in total.\n";
	std::cout << BOLDBLUE << "You make " << TransitorsString(calcTPS(), 1) << " per second.\n";

	CMD::log("They have " + TransitorsString(gameState.transistorBalance, 0, "", "") + "!");
}

void buy(std::vector<str>& args)     {
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
			
			Upgrade& upgrade = upgrades[getUpgradeByName(Uname)];

			if(!upgrade.unlocked) {
				std::cout << BOLDRED << "Unkown upgrade!\n";
				return;
			}

			if(upgrade.purchased) {
				std::cout << BOLDRED << "You already have this upgrade!\n";
				return;
			}

			if(gameState.transistorBalance >= upgrade.cost) {
				gameState.transistorBalance -= upgrade.cost;
				upgrade.purchased = true;

				std::cout << BOLDBLUE << "You bought " << upgrade.name << " for " << TransitorsString(upgrade.cost) << ".\n";
				return;
			} else {
				std::cout << BOLDRED << "You don't have enough transistors to buy " << upgrade.name << '\n';
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
			countPtr = &gameState.cusors;
			buildingUnlocked = gameState.cursorUnlocked;
		    break;
		case moss:
		    buildingName = "moss";
		    basePrice = mossPrice;
		    countPtr = &gameState.moss;
			buildingUnlocked = gameState.mossUnlocked;
		    break;
		case smallFAB:
		    buildingName = "small FAB";
		    basePrice = smallFABPrice;
		    countPtr = &gameState.smallFABs;
			buildingUnlocked = gameState.smallFABUnlocked;
		    break;
		case mediumFAB:
		    buildingName = "medium FAB";
		    basePrice = mediumFABPrice;
		    countPtr = &gameState.mediumFABs;
			buildingUnlocked = gameState.mediumFABUnlocked;
		    break;
		case largeFAB:
		    buildingName = "large FAB";
		    basePrice = largeFABPrice;
		    countPtr = &gameState.largeFABs;
			buildingUnlocked = gameState.largeFABUnlocked;
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

			while(gameState.transistorBalance >= finalPrice) {
				totalBought++;
				finalPrice += expandPrice(basePrice, count + totalBought);
			}

			totalBought--;
			finalPrice -= expandPrice(basePrice, count + totalBought);

			count += totalBought;
			gameState.transistorBalance -= finalPrice;

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

		if(gameState.transistorBalance >= totalPrice) {
			gameState.transistorBalance -= totalPrice;
			count += toPurchase;

			std::cout << BOLDBLUE << "bought " << numString(toPurchase, buildingName) << " for " << TransitorsString(totalPrice) << "!\n";
		} else {
			std::cout << BOLDRED << "You don't have enough transistors to buy " << toPurchase << " for " << TransitorsString(totalPrice, 0, BOLDRED, BOLDRED) << "!\n";
		}
	} else {
		if(gameState.transistorBalance >= expandPrice(basePrice, count)) {
			gameState.transistorBalance -= expandPrice(basePrice, count);
			count++;
		} else {
			std::cout << BOLDRED << "You don't have enough transistors to buy a " << buildingName << "!\n";
			return;
		}

		std::cout << BOLDBLUE << "bought a " << buildingName << " for " << TransitorsString(expandPrice(basePrice, count - 1)) << "!\n";
	}
}

void info(std::vector<str>& args)    {
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
				std::cout << RESET << "Makes " << TransitorsString(calcCursorYeild() * 10, 0, "", "") << " every ten seconds\n\n";
				std::cout << BOLDWHITE << "\"A nice cusor for your nice mouse\"\n";
			} else if(args[1] == "moss") {
				std::cout << BOLDWHITE << "MOSS\n";
				std::cout << BOLDCYAN << "Description:\n";
				std::cout << RESET << "Some moss to... somthing more transistors\n";
				std::cout << BOLDCYAN << "Effect:\n";
				std::cout << RESET << "Makes " << TransitorsString(calcMossYeild(), 0, "", "") << " every second\n\n";
				std::cout << BOLDWHITE << "\"Some nice moss to uh..... cookies\"\n";
			} else if(args[1] == "smallfab") {
				std::cout << BOLDWHITE << "SMALL FAB\n";
				std::cout << BOLDCYAN << "Description:\n";
				std::cout << RESET << "Small FABs make microchips which contain transistors\n";
				std::cout << BOLDCYAN << "Effect:\n";
				std::cout << RESET << "Makes " << TransitorsString(calcSmallFABYeild(), 0, "", "") << " every second\n\n";
				std::cout << BOLDWHITE << "\"Self-containing!\"\n";
			} else if(args[1] == "mediumfab") {
				std::cout << BOLDWHITE << "MEDIUM FAB\n";
				std::cout << BOLDCYAN << "Description:\n";
				std::cout << RESET << "A faster fab to make more transistors\n";
				std::cout << BOLDCYAN << "Effect:\n";
				std::cout << RESET << "Makes " << TransitorsString(calcMediumFABYeild(), 0, "", "") << " every second\n\n";
				std::cout << BOLDWHITE << "\"This is AVERAGE!?! MORE PRODUCTION!\"\n";
			} else if(args[1] == "largefab") {
				std::cout << BOLDWHITE << "LARGE FAB\n";
				std::cout << BOLDCYAN << "Description:\n";
				std::cout << RESET << "A yet larger FAB to make transistors at an even faster rate\n";
				std::cout << BOLDCYAN << "Effect:\n";
				std::cout << RESET << "Makes " << TransitorsString(calcLargeFABYeild(), 0, "", "") << " every second\n\n";
				std::cout << BOLDWHITE << "\"???\"\n";
			} else {
				std::cout << BOLDRED << "Unknown building!\n";
			}
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

			Upgrade& upgrade = upgrades[getUpgradeByName(Uname)];

			if(!upgrade.unlocked) {
				std::cout << BOLDRED << "Unkown upgrade!\n";
				return;
			}
			
			std::cout << BOLDWHITE << "UPGRADE: " << upgrade.name << '\n';
			std::cout << BOLDCYAN  << "Description:\n";
			std::cout << RESET     << upgrade.description << '\n';
			std::cout << BOLDCYAN  << "Effect:\n";
			std::cout << RESET     << upgrade.effect << "\n\n";
			std::cout << BOLDWHITE << '"' << upgrade.flavortext << "\"\n";
		} else if(args[0] == "achievement") {
			str Aname = "";

			for(size_t i = 1; i < args.size(); i++) {
				Aname += args[i];
				if(i != args.size() - 1) Aname += " ";
			}

			if(achievementIndex.find(tolower(Aname)) == achievementIndex.end()) {
				std::cout << BOLDRED << "Unkown achievement";
				return;
			}

			Achievement& achievement = achievements[getAchievementByName(Aname)];

			if(tolower(Aname) == "all you had to do was ask") {
				std::cout << "You found me!\n\n";
				achievement.earned = true;
			}

			if(!achievement.earned) {
				std::cout << RED << "What!?! You haven't earned this, SCRAM!";
				return;
			}

			if(achievement.name == "fabrication") {
				achievement.name = "FABrication";
			}

			std::cout << BOLDWHITE << "ACHIEVEMENT: "  << achievement.name << '\n';
			std::cout << BOLDCYAN  << "Description: \n";
			std::cout << RESET     << achievement.description << "\n\n";
			std::cout << BOLDWHITE << '"' << achievement.flavortext << "\"\n";
		}
	}
}

void help(std::vector<str>& args)    {
	if(args.size() < 1) {
		std::cout << BOLDBLUE << "Commands:\n";
		std::cout << BOLDBLUE << "balance " << RESET << " - Shows info about your transistor balance\n";
		std::cout << BOLDBLUE << "buy     " << RESET << " - Buys the selected item\n";
		std::cout << BOLDBLUE << "clear   " << RESET << " - Clears the console\n";
		std::cout << BOLDBLUE << "help    " << RESET << " - Gives assistance\n";
		std::cout << BOLDBLUE << "list    " << RESET << " - Lists things\n";
		std::cout << BOLDBLUE << "notes   " << RESET << " - Shows unattended achievements\n";
		std::cout << BOLDBLUE << "info    " << RESET << " - Shows info as selected thing\n";
		std::cout << BOLDBLUE << "save    " << RESET << " - Saves game\n";
		#ifdef DEBUG
		std::cout << BOLDBLUE << "bHash   " << RESET << " - Lists building hashes\n";
		#endif
	} else if(args.size() >= 1) {
			   if(args[0] == "balance") {
			if(args.size() == 1) {
			std::cout << BOLDWHITE << "BALANCE\n";
			std::cout << RESET << "Show quantitative info about specified thing\n";
			std::cout << BOLDCYAN << "\nUsage:\n";
			std::cout << RESET << "balance [thing]\n";
			std::cout << BOLDCYAN << "\nThings:\n";
			std::cout << BOLDBLUE << "defualt" << RESET << " - transistor\n";
			std::cout << BOLDCYAN << "Subcommands:\n";
			std::cout << BOLDBLUE << "building" << RESET << " - Shows quantitative info about a building\n";
			} else if(args.size() >= 2) {
				if(args[1] == "building") {
					std::cout << BOLDWHITE << "BALANCE BUILDING\n";
					std::cout << RESET << "Show quantitative info about a building\n";
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
		} else if(args[0] == "list") {
			if(args.size() == 1) {
			std::cout << BOLDWHITE << "LIST\n";
			std::cout << RESET << "Lists things, if none is specified then it tells you it can't list noting\n";
			std::cout << BOLDCYAN << "\nUsage:\n";
			std::cout << RESET << "list [thing]\n";
			std::cout << BOLDCYAN << "\nThings:\n";
			std::cout << BOLDBLUE << "building" << RESET << " - lists buildings\n";
			std::cout << BOLDBLUE << "achievement" << RESET << " - lists achievements\n";
			std::cout << BOLDCYAN << "Subcommands:\n";
			std::cout << BOLDBLUE << "upgrade | upgrades" << RESET << " - lists owned upgrades\n";
			} else if(args.size() >= 2) {
				if(args[1] == "upgrade" || args[1] == "upgrades") {
					std::cout << BOLDWHITE << "LIST UPGRADES\n";
					std::cout << RESET << "Lists upgrades\n";
					std::cout << BOLDCYAN << "\nThings:\n";
					std::cout << BOLDBLUE << "all" << RESET << " - lists all upgrades\n";
					std::cout << BOLDBLUE << "owned" << RESET << " - lists owned upgrades\n";
				}
			}
		} else if(args[0] == "notes") {
			std::cout << BOLDWHITE << "CLEAR\n";
			std::cout << RESET << "Shows unattended notifications, then clears them\n";
			std::cout << BOLDCYAN << "\nUsage:\n";
			std::cout << RESET << "notes\n";
		} else if(args[0] == "info") {
			std::cout << BOLDWHITE << "INFO\n";
			std::cout << RESET     << "Shows qualitative information about things in the game\n";
			std::cout << BOLDCYAN  << "\nUsage:\n";
			std::cout << RESET     << "info [thing]\n";
			std::cout << BOLDCYAN  << "\nThings:\n";
			std::cout << RESET     << "Inummerable\n";
			std::cout << BOLDCYAN  << "Subcommands:\n";
			std::cout << BOLDBLUE  << "building"    << RESET << " - Shows info about selected building\n";
			std::cout << BOLDBLUE  << "upgrade"     << RESET << " - Shows info about selected upgrade\n";
			std::cout << BOLDBLUE  << "achievement" << RESET << " - Shows info about selected achievement\n";
		} 
		#ifdef DEBUG 
		else if(args[0] == "bHash") {
			std::cout << BOLDWHITE << "BHASH\n";
			std::cout << RESET << "lists building hashes\n";
			std::cout << BOLDCYAN << "Usage:\n";
			std::cout << RESET << "bHash\n";
		}
		#endif
	}
}

void list(std::vector<str>& args)    {
	if(args.size() < 1) {
		std::cout << BOLDRED << "You must specify some group of things to list!\n";
	} else if(args.size() >= 1) {
			   if(args[0] == "upgrade" || args[0] == "upgrades") {
			if(args.size() == 1) {
				std::cout << BOLDWHITE << "Upgrades:\n\n";

				for(Upgrade& upgrade : upgrades) {
					if(upgrade.unlocked && !upgrade.purchased) {
						std::cout << BOLDBLUE << upgrade.name << " - " << upgrade.description << '\n';
						std::cout << "it will cost you " << TransitorsString(upgrade.cost, 0) << "!\n\n";
					}
				}
			} else if(args.size() >= 2) {
				if(args[1] == "owned") {
					std::cout << BOLDWHITE << "Owned upgrades:\n\n";

					for(Upgrade& upgrade : upgrades) {
						if(upgrade.purchased) {
							std::cout << BOLDBLUE << upgrade.name << " - " << upgrade.description << BOLDYELLOW << " (owned)\n";
							std::cout << BOLDBLUE << "cost " << TransitorsString(upgrade.cost, 0) << "!\n\n";
						}
					}
				} else if(args[1] == "all") {
					std::cout << BOLDWHITE << "All upgrades:\n\n";

					std::vector<Upgrade> owned;
					std::vector<Upgrade> unlocked;

					for(Upgrade& upgrade : upgrades) {
						if(upgrade.unlocked) {
							if(upgrade.purchased) {
								owned.push_back(upgrade);
							} else {
								unlocked.push_back(upgrade);
							}
						}
					}

					for(Upgrade& upgrade : owned) {
						std::cout << BOLDBLUE << upgrade.name << " - " << upgrade.description << BOLDYELLOW << " (owned)\n";
						std::cout << BOLDBLUE << "will cost " << TransitorsString(upgrade.cost) << "!\n\n";
					}

					for(Upgrade& upgrade : unlocked) {
						std::cout << BOLDBLUE << upgrade.name << " - " << upgrade.description << '\n';
						std::cout << "cost " << TransitorsString(upgrade.cost) << "!\n\n";
					}
				}
			}
		} else if(args[0] == "achievement" || args[0] == "achievements") {
			if(args.size() >= 1) {
				std::cout << BOLDWHITE << "Achievments:\n\n";

				for(Achievement& achievement : achievements) {
					if(achievement.name == "fabrication") {
						achievement.name = "FABrication";
					}
					
					if(achievement.earned) {
						std::cout << BOLDBLUE << achievement.name << " - " << achievement.description << '\n';
						std::cout << BOLDWHITE << '"' << achievement.flavortext << "\"\n\n";
					}
				}
			}
		} else if(args[0] == "building" || args[0] == "buildings") {
			std::cout << BOLDWHITE << "Buildings:\n\n";

			if(gameState.cursorUnlocked) {
				std::cout << BOLDBLUE << "Cursor - " << "Autoclicks once every 10 seconds\n";
				std::cout << "one will cost " << TransitorsString(expandPrice(cursorPrice, gameState.cusors)) << "!\n\n";
			}

			if(gameState.mossUnlocked) {
				std::cout << BOLDBLUE << "Moss - " << "Some moss to... somthing more transistors\n";
				std::cout << "one will cost " << TransitorsString(expandPrice(mossPrice, gameState.moss)) << "!\n\n";
			}

			if(gameState.smallFABUnlocked) {
				std::cout << BOLDBLUE << "Small FAB - " << "Small FABs make microchips which contain transistors\n";
				std::cout << "one will cost " << TransitorsString(expandPrice(smallFABPrice, gameState.smallFABs)) << "!\n\n";
			}

			if(gameState.mediumFABUnlocked) {
				std::cout << BOLDBLUE << "Medium FAB - " << "A faster fab to make more transistors\n";
				std::cout << "one will cost " << TransitorsString(expandPrice(mediumFABPrice, gameState.mediumFABs)) << "!\n\n";
			}

			if(gameState.largeFABUnlocked) {
				std::cout << BOLDBLUE << "Large FAB - " << "A yet larger FAB to make transistors at an even faster rate\n";
				std::cout << "one will cost " << TransitorsString(expandPrice(largeFABPrice, gameState.mediumFABs)) << "!\n\n";
			}
		}
	}
}

void notes(std::vector<str>& args)   {
	for(Notification notification : notifications) {
		std::cout << BOLDYELLOW << notification.title <<     '\n';
		std::cout << BOLDWHITE  << notification.content << "\n\n";
	}

	notifications.clear();
}

void save(std::vector<str>& args)    {
	if(gameName == "") {
		std::cout << BOLDRED << "This is invalid state, HACKER!";
	}

	std::cout << BOLDWHITE << "SAVING...\n";

	saveGame("save/" + gameName + ".json");

	std::cout << BOLDGREEN << "Done!\n";
}

#ifdef DEBUG

void bHash(std::vector<str>& args) {
	str buildings[] = {"cursor", "moss", "smallFAB", "mediumFAB", "largeFAB", "i860", "startup", "oaktree"};

	for(int i = 0; i < 8; i++) {
		std::stringstream ss;
		ss << buildings[i] << ":" << hash(tolower(buildings[i]));
		CMD::log(ss.str());
	}
}

#endif

#pragma endregion

int main() {
	GRP::init();
	
	bool exited = false;

	bool skipped = false;

	createUpgrade("integrated mouse", "The mouse now integrates semiconductor technology into it's design", "doubles mouse and cursor output.", "Now with semiconductor technology!", 100, cursorUpgrades, {canUnLockIntegratedMouse, unLockIntegratedMouse});
	createUpgrade("faster fingers", "Makes fingers faster", "doubles mouse and cursor output.", "Buy our finger speed pills today, double finger speed garauntee!", 500, cursorUpgrades, {canUnLockFastFing, unLockFastFing});
	createUpgrade("chippy", "An assistant to improve your clicking methods.", "doubles mouse and cursor output.", "do you want to click a button? how about writing a letter instead.", 10'000, cursorUpgrades, {canUnLockChippy, unLockChippy});

	createUpgrade("mossy mossy", "Boosts moss production... somehow.", "doubles moss output.", "Mossy Mossy", 1'000, mossUpgrades, {canUnLockMossyMossy, unLockMossyMossy});
	createUpgrade("moss walls", "Add moss walls to the office space", "doubles moss output.", "Add moss walls to the halls so the moss has more space to grow.", 5'000, mossUpgrades, {canUnLockMossWalls, unLockMossWalls});

	createUpgrade("cheap lithography machines", "Makes small FABs better!", "doubles small FAB output", "Cheaper lithography machines make small FABs more cost effective.", 10'000, smallFABUpgrades, {canUnLockCheapMachines, unLockCheapMachines});
	createUpgrade("denser chips", "More transistors fit on the same chip!", "doubles small FAB output.", "All new chipmaking process doubles chip density and performance!", 50'000, smallFABUpgrades, {canUnLockDenseChips, unLockDenseChips});

	createUpgrade("mossier tech", "Includes moss in microchips.", "doubles small & meduim FAB output", "Whitness the pure power of all new MOSS transistors", 110'000, mediumFABUpgrades, {canUnLockMossyTech, unLockMossyTech});

	createUpgrade("endgame", "The game is complete...\n\nfor now", "doubles all FABs output", "huh I thought there'd be more", 200'000, largeFABUpgrades, {canUnLockEndgame, unLockEndgame});

	createAchievement("click", "Press enter without entering a command", "clack", {canUnLockClick, unLockClick});
	createAchievement("semigood", "Earn 1,000 transistors", "kinda yummy", {canUnLockSemigood, unLockSemigood});
	createAchievement("transistant", "Earn 100,000 transistors", "I'm trancen-I mean transisting", {canUnLockTransistant, unLockTransistant});

	createAchievement("casual semiconductors", "Make 1 transistor a second", "YOY!", {canUnLockCasualSemi, unLockCasualSemi});
	createAchievement("hardcore semiconductors", "Make 10 transistors per second", "Unachievable!", {canUnLockHarcoreSemi, unLockHarcoreSemi});
	createAchievement("steady semistream", "Make 100 transistors per second", "Truly unachievable!", {canUnLockSteadyStream, unLockSteadyStream});

	createAchievement("autoclick", "Own 1 cursor", "autoclack", {canUnLockAutoClick, unLockAutoClick});
	createAchievement("double click", "Own 2 cursors", "click clock", {canUnLockDoubleClick, unLockDoubleClick});
	createAchievement("mouse wheel", "Own 50 cursors", "spin, oops I forgot!", {canUnLockMouseWheel, unLockMouseWheel});

	createAchievement("mossy", "Own 1 moss", "Mossy M-Ooops spoilers!", {canUnLockMossy, unLockMossy});
	createAchievement("moss gardens", "Own 50 moss", "but moss needs more MORE MOSS!", {canUnLockMossGardens, unLockMossGardens});

	createAchievement("FABrication", "Own 1 small FAB", "FABulous!", {canUnLockFABrication, unLockFABrication});

	#ifdef DEBUG
	str dataMineEnd = "I'll try the datamine command, maybe I get it easy with my debug build";
	#else
	str dataMineEnd = "shucks I have only have a release build";
	#endif

	createAchievement("datamining", "Own 1 medium FAB", "hmmmmm, is it javascrpt.datamine(), oh wait, this is C++ " + dataMineEnd, {canUnLockDatamining, unLockDatamining});

	createAchievement("that's big", "Own 1 large FAB", "your big, NERD!", {canUnLockThatsBig, unLockThatsBig});

	createAchievement("all you had to do was ask", "really that's all", "ask and you shall acheive!", {alwayTrue, duZNutin});

	createAchievement("???", "???", "??? You know it gets bad when the flavortext is \"???\"", {canUnLockFinal, unLockFinal});

	while(!exited) {

		if(!skipped) 
		std::cout << BOLDBLUE << name << ' ' << BOLDGREEN << version << RESET << '\n';
		std::cout << YELLOW << "Save & Load Your Upgrades & Achievements\n\n" << RESET;

		skipped = true;

		str action;

		std::cout << BOLDYELLOW << "Actions:\n\n";

		std::cout << BOLDGREEN << "n" << RESET << " - New game\n";
		std::cout << BOLDGREEN << "l" << RESET << " - Load game\n";
		std::cout << BOLDGREEN << "d" << RESET << " - Delete game\n";
		std::cout << BOLDGREEN << "c" << RESET << " - Copy save\n";
		std::cout << BOLDGREEN << "m" << RESET << " - Move save\n";
		std::cout << BOLDGREEN << "e" << RESET << " - Exit\n\n";

		std::cout << "Action: ";

		getline(std::cin, action);

			   if(action == "n") {
			gameState = {0, 0, 0, 0, 0, 0, 0, 0, false, false, false, false, false};

naming:
			std::cout << "\nGame name: ";

			getline(std::cin, gameName);

			if(gameName == "" || gameName.back() == ' ' || gameName == "saveindex") {
				std::cout << BOLDRED << "Please enter valid name" << RESET;
				goto naming;
			}

			if(std::filesystem::exists("save/" + gameName + ".json")) {
				std::cout << BOLDRED << "Save already exists\n" << RESET;
				goto naming;
			}

			std::ofstream save("save/" + gameName + ".json");
			save << "{}";

			std::ifstream indexFile("save/saveindex.json");

			json index = json::parse(indexFile);

			index["saves"].push_back(gameName);

			std::remove("save/saveindex.json");

			std::ofstream indexFileWrite("save/saveindex.json");

			indexFileWrite << index.dump(4) << std::flush;

			std::cout << index.dump(4);
		} else if(action == "l") {
			json index;
			if(std::filesystem::exists("save/saveindex.json")) {
				std::ifstream indexFile("save/saveindex.json");
				index = json::parse(indexFile);
			} else {
				index = json::parse("{}");
			}

select:
			std::cout << BOLDYELLOW << "\nSaves:\n";

			json saveNames = index["saves"];

			for(str name : saveNames) {
				std::cout << BOLDWHITE << name << '\n';
			}

			std::cout << RESET     << "\nSave name: ";

			getline(std::cin, gameName);

			if(gameName == "" || gameName.back() == ' ' || gameName == "saveindex") {
				std::cout << BOLDRED << "Please enter valid name\n\n" << RESET;
				goto select;
			}

			if(!std::filesystem::exists("save/" + gameName + ".json")) {
				std::cout << BOLDRED << "Save do not exist\n\n" << RESET;
				goto select;
			}

			loadGame("save/" + gameName + ".json");
		} else if(action == "d") {
			str delName;

			json index;
		   if(std::filesystem::exists("save/saveindex.json")) {
			   std::ifstream indexFile("save/saveindex.json");
			   index = json::parse(indexFile);
		   } else {
				index = json::parse("{}");
		   }

			std::cout << BOLDYELLOW << "\nSaves:\n";

			json& saveNames = index["saves"];

			for(str name : saveNames) {
				std::cout << BOLDWHITE << name << '\n';
			}

			std::cout << RESET << "\nSave name: ";

			getline(std::cin, delName);

			if(delName == "" || delName.back() == ' ' || delName == "saveindex") {
				std::cout << BOLDRED << "Please enter valid name\n\n" << RESET;
				goto rerun;
			}

			if(!std::filesystem::exists("save/" + delName + ".json")) {
				std::cout << BOLDRED << "Save does not exist\n\n" << RESET;
				goto rerun;
			}

			str sure;

			std::cout << "\nAre you sure [y/N]?";

			std::getline(std::cin, sure);

			if(tolower(sure) != "y") {
				goto rerun;
			}

			std::filesystem::remove("save/" + delName + ".json");

			saveNames.erase(saveNames.find(delName));

			std::remove("save/saveindex.json");

			std::ofstream indexFileWrite("save/saveindex.json");

			indexFileWrite << index.dump(4) << std::flush;

			std::cout << RED << "\nDeleted\n\n";

			goto rerun;
		} else if(action == "c") {
			str delName;

			json index;
		    if(std::filesystem::exists("save/saveindex.json")) {
				std::ifstream indexFile("save/saveindex.json");
			    index = json::parse(indexFile);
		    } else {
				index = json::parse("{}");
		    }

			std::cout << BOLDYELLOW << "\nSaves:\n";

			json& saveNames = index["saves"];

			for(str name : saveNames) {
				std::cout << BOLDWHITE << name << '\n';
			}

			std::cout << RESET << "\nSave name: ";

			getline(std::cin, delName);

			if(delName == "" || delName.back() == ' ' || delName == "saveindex") {
				std::cout << BOLDRED << "Please enter valid name\n\n" << RESET;
				goto rerun;
			}

			if(!std::filesystem::exists("save/" + delName + ".json")) {
				std::cout << BOLDRED << "Save does not exist\n\n" << RESET;
				goto rerun;
			}

			str cpyTo;

			std::cout << RESET << "\nSave destination: ";

			getline(std::cin, cpyTo);

			std::cout << GREEN << "\nCopying...\n\n";

			saveNames.push_back(cpyTo);

			std::ifstream src("save/" + delName + ".json", std::ios::binary);
    		std::ofstream dst("save/" + cpyTo   + ".json", std::ios::binary);

    		dst << src.rdbuf();

			src.close();
			dst.close();

			std::remove("save/saveindex.json");

			std::ofstream indexFileWrite("save/saveindex.json");

			indexFileWrite << index.dump(4) << std::flush;

			goto rerun; 
		} else if(action == "m") {
			str delName;

			json index;
		    if(std::filesystem::exists("save/saveindex.json")) {
				std::ifstream indexFile("save/saveindex.json");
			    index = json::parse(indexFile);
		    } else {
				index = json::parse("{}");
		    }

			std::cout << BOLDYELLOW << "\nSaves:\n";

			json& saveNames = index["saves"];

			for(str name : saveNames) {
				std::cout << BOLDWHITE << name << '\n';
			}

			std::cout << RESET << "\nSave name: ";

			getline(std::cin, delName);

			if(delName == "" || delName.back() == ' ' || delName == "saveindex") {
				std::cout << BOLDRED << "Please enter valid name\n\n" << RESET;
				goto rerun;
			}

			if(!std::filesystem::exists("save/" + delName + ".json")) {
				std::cout << BOLDRED << "Save does not exist\n\n" << RESET;
				goto rerun;
			}

			str cpyTo;

			std::cout << RESET << "\nSave destination: ";

			getline(std::cin, cpyTo);

			std::cout << GREEN << "\nMoving...\n\n";

			saveNames.erase(saveNames.find(delName));

			saveNames.push_back(cpyTo);

			std::ifstream src("save/" + delName + ".json", std::ios::binary);
    		std::ofstream dst("save/" + cpyTo   + ".json", std::ios::binary);

    		dst << src.rdbuf();

			src.close();
			dst.close();

			std::filesystem::remove("save/" + delName + ".json");

			std::remove("save/saveindex.json");

			std::ofstream indexFileWrite("save/saveindex.json");

			indexFileWrite << index.dump(4) << std::flush;

			goto rerun; 
		} else if(action == "e") {
			break;
			exited = true;
		} else {
			std::cout << BOLDRED << "\nInvalid action!\n";
			std::this_thread::sleep_for(std::chrono::seconds(10));
			goto rerun;
		}

		{
		std::jthread gameThread = CMD::init(name, BOLDGREEN + str("@HCC") + BOLDBLUE + " ~/You" + RESET + "$ ", onTick);

		CMD::exit = onExit;

		CMD::addcommand("balance", balance);
		CMD::addcommand("buy"    , buy    );
		CMD::addcommand("clear"  , clear  );
		CMD::addcommand("help"   , help   );
		CMD::addcommand("info"   , info   );
		CMD::addcommand("list"   , list   );
		CMD::addcommand("notes"  , notes  );
		CMD::addcommand("save"   , save   );
		#ifdef DEBUG
		CMD::addcommand("bHash", bHash);
		#endif

		addUpgradeTriggers();
		addAchievementTriggers();

		addTrigger({isCursorUnLocked, unlockCursor});
		addTrigger({isMossUnLocked, unlockMoss});
		addTrigger({isSmallFABUnLocked, unlockSmallFAB});
		addTrigger({isMediumFABUnLocked, unlockMediumFAB});
		addTrigger({isLargeFABUnLocked, unlockLargeFAB});

		CMD::log("Program iniitialized");

		CMD::runcomm("clear", click);

		CMD::command_loop(click);

		CMD::kill(gameThread);

		triggers.clear();

		}
		skipped = false;
rerun:
		int useless = 0;
	}

	return 0;
}
