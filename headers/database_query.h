#ifndef DATABASE_QUERY_H
#define DATABASE_QUERY_H

#include <string>

#include "database.h"
#include "submission_information.h"
#include "testlib_message.h"

class MDatabaseQuery
{
public:
	//MDatabaseQuery() {};
	enum DataStructure {FILES = 0, MAGIC_IVAN = 1};
	MDatabaseQuery(std::string aDatabasePath);
	~MDatabaseQuery();
	
	//takeProblemId(sqlite3* ppDb);
	//coutRows(sqlite3* ppDb, int problemNum);
	//takeProblemId(sqlite3* ppDb, int problemNum);

	void prepareForTesting(SubmissionInformation& aSudmissionInformation, 
		DataStructure aDataType = DataStructure::FILES);
	void writeResult(int aID, std::string aResult, int aTime, int aMemory);

	void getNextTest(SubmissionInformation& aSudmissionInformation, TestLibMessage& aTLM);
private:
	Database mDatabase;
	int mReservedStatementNumber;

	void prepareTestsStatement(SubmissionInformation& aSudmissionInformation);

	void getIDInformation(SubmissionInformation& aSudmissionInformation);
	void getLimitsInformation(SubmissionInformation& aSudmissionInformation);
	void getTests(SubmissionInformation& aSudmissionInformation);
};

#endif