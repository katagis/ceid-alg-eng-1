#include "LEDA/graph/graph.h"
#include "LEDA/core/list.h"

#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <string>
#include <set>


#ifdef USE_CHRONO
#include <chrono>
namespace ch = std::chrono;
#else 
// Chrono is not available on g++ 4.4.7
// Use Unix Time, sometimes fails but its the only timer that provides proper microseconds
#include <sys/time.h>
#endif

bool GVerbose = false;

using leda::list;
using leda::graph;
using leda::node;

// Required for the test function
bool MyIsBipartite(const graph& Graph, list<node>& PartA, list<node>& PartB);


// Struct to hold the benchmark results.
// Implementation can switch between chrono / unix time through defining USE_CHRONO.
// Chrono is a bit more accurate but not available on older c++ versions.
// All time values (stored as long long) are in microseconds.
struct Benchmark {
private:
	std::vector<long long> MyTime;
	std::vector<long long> LedaTime;

#ifdef USE_CHRONO
	ch::time_point<ch::system_clock> StartTime;
	void RestartTimer() {
		StartTime = ch::system_clock::now();
	}

	long long GetCurrent() const {
		return ch::duration_cast<ch::microseconds>(ch::system_clock::now() - StartTime).count();
	}
#else
	long StartTime;

	long GetUnixMicros() const {
		struct timeval TimeVal;
		struct timezone TimeZone;
		if (gettimeofday(&TimeVal, &TimeZone)) {
            return 0;
        }
		return TimeVal.tv_usec;
	}

	void RestartTimer() {
		StartTime = GetUnixMicros();
	}

	long long GetCurrent() const {
        long UnixMicros = GetUnixMicros();
        if (UnixMicros <= 0 || StartTime <= 0) {
            return 0;
        }
        return UnixMicros - StartTime;
	}
#endif

	// Internal,  formats and prints a line with 2 times and their difference.
	void PrintBenchLine(long long MyT, long long LedaT) {
		static std::string TimestepStr = " \u03BCs";

		std::cout << " | Mine: " << std::setw(7) << MyT << TimestepStr
				 << " | Leda: " << std::setw(7) << LedaT << TimestepStr;

        if (MyT <= 0 || LedaT <= 0) {
		    std::cout << "\t- Invalid timer result detected. -\n";
            return;
        }

		long long Hi = std::max(MyT, LedaT);
		long long Lo = std::max(std::min(MyT, LedaT), 1LL);
		
		int Percent = std::floor(((float)Hi / Lo) * 100.f + 0.5f) - 100;
		long long AbsDiff = Hi - Lo;

		std::string Who = MyT < LedaT ? "Mine" : "Leda";
		std::cout << "\t" << Who << " is faster by: " << std::setw(3) << Percent << "% (" << std::setw(7) << AbsDiff << TimestepStr << ")\n" ;
	}

public:

	void Reset() {
		MyTime.clear();
		LedaTime.clear();
	}

	void StartTest() {
		RestartTimer();
	}

	// The leda test has finished and my test is starting
	void SwitchTest() {
		long long Duration = GetCurrent();
		LedaTime.push_back(Duration);
		RestartTimer();
	}

    void StopTest() {
		long long Duration = GetCurrent();
		MyTime.push_back(Duration);
	}

	// Print the last added test.
	void PrintLast() {
		size_t Index = MyTime.size() - 1;
		PrintBenchLine(MyTime[Index], LedaTime[Index]);
	}

	// Calculate and print total stats.
	void Print() {
        bool ContainsInvalidResult = false;
		long long MyTotal = 0;
		long long LedaTotal = 0;
		for (int i = 0; i < MyTime.size(); ++i) {
            if (MyTime[i] > 0 && LedaTime[i] > 0) {
                MyTotal += MyTime[i];
                LedaTotal += LedaTime[i];
            }
            else {
                ContainsInvalidResult = true;
            }
		}

		std::cout << "Totals:";
		PrintBenchLine(MyTotal, LedaTotal);
        std::cout << "\n";

        if (ContainsInvalidResult) {
            std::cout << "Unix timer does not always guarantee returning a valid time.\n"
                    << "It is HIGHLY recommended to use chrono if possible for better results.\n"
                    << "Results detected with invalid times where not included in the total.\n\n"
                    << "Rerunning the program usually fixes this.\n";
        }
	}
};


static Benchmark Bench;


// Compares the contents of 2 lists, returns true even if the order is different
// This modifies the lists but thats fine for our test case.
bool IsListSame(list<node>& ListA, list<node>& ListB) {
	if (ListA.size() != ListB.size()) {
		return false;
	}

	ListA.sort();
	ListB.sort();
	

	list<node>::iterator ItA;
	list<node>::iterator ItB;

	// Both iterations are guaranteed to end at the same time because the lists are confirmed to have
	// the same size
	for (ItA = ListA.begin(), ItB = ListB.begin(); ItA; ++ItA, ++ItB) {
		if (*ItA != *ItB) {
			return false;
		}
	}

	return true;
}



bool TestGraph(const graph& Graph, int TestNum = -1, const std::string& TestName = "") {
	if (Graph.empty()) {
		return true;
	}

	list<node> MyA, MyB;
	list<node> LedaA, LedaB;
	bool MyResult;
	bool LedaResult;

	
	Bench.StartTest();
	LedaResult = leda::Is_Bipartite(Graph, LedaA, LedaB);
	Bench.SwitchTest();
	MyResult = MyIsBipartite(Graph, MyA, MyB);
	Bench.StopTest();


	bool SameResult = MyResult == LedaResult;
	
	// If both results are false, skip checking the list
	bool TestResult = SameResult && (MyResult == false || (IsListSame(MyA, LedaA) && IsListSame(MyB, LedaB)));

	if (GVerbose) {
		std::cout << "Ret\t> My: " << MyResult << " | Leda: " << LedaResult << "\n";
		
		// If both calculated false we should just not print anything
		if (MyResult || LedaResult) {
			std::cout << "Size A\t> My: " << MyA.size() << " | Leda: " << LedaA.size() << "\n";
			std::cout << "Size B\t> My: " << MyB.size() << " | Leda: " << LedaB.size() << "\n";


			std::cout << "MyA  : " << MyA << "\n";
			std::cout << "LedaA: " << LedaA << "\n";

			std::cout << "MyB  : " << MyB << "\n";
			std::cout << "LedaB: " << LedaB << "\n";
		}
	}


	const int MaxNameLen = 16;
	std::ostringstream PaddedName;
	PaddedName <<  ": " << TestName.substr(0, MaxNameLen - 2);
	std::cout << "# Test " << std::setw(2) << TestNum 
			<< std::left << std::setw(MaxNameLen) << PaddedName.str() << "| " << std::right; 
	
	if (TestResult){
		std::string ResultStr = MyResult == true ? "yes" : "no ";
		std::cout << ResultStr;
		Bench.PrintLast();
	}
	else {
		std::cout << "# Test failed. Results are different.\n";
	}

	return TestResult;
}
