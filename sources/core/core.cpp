#include "core/core.hpp"

//--------------------------------------------------------------------------------
//				CHILD PROCESS CREATION IMPLIMENTATION 
//--------------------------------------------------------------------------------


#undef max
#undef min

#include <cmath>

#define THREAD_COUNTS 1
#define DEBUG_PLUS_SOLUTION_SUBMISSION 1

#if DEBUG_PLUS_SOLUTION_SUBMISSION
#define _DEBUG_SOL_SUB_
#endif

Core::Core
(
    std::string aDatabasePath
) :
    mDBQ            (aDatabasePath),
    mChecksMutexs   (THREAD_COUNTS),
    mChecksInfo     (THREAD_COUNTS),
    mFinishedTest   (0)
{
    mProblemInfo.mResult = "ok";
    for (uint_32 i = 0; i < THREAD_COUNTS; ++i)
    {
        mChecks.push_back(new std::thread([](){}));
    }

}

Core::~Core()
{
    for (uint_32 i = 0; i < mChecks.size(); ++i)
    {
        if (mChecks[i] != NULL)
        {
            mChecks[i]->join();
            delete mChecks[i];
        }
    }
}

void
Core::run(int aID){
    mProblemInfo = mDBQ.(aID);

// #ifdef BILL_WINDOWS
//     makeWindowString(mProblemInfo.mSolutionFileName);
//     makeWindowString(mProblemInfo.mCheckerFileName);
// #endif // BILL_WINDOWS
    mProblemInfo.mSolutionFileName.clear();

#ifdef _DEBUG_SOL_SUB_
#if  defined(BILL_WINDOWS)
    mProblemInfo.mSolutionFileName = "2\\plus.cpp";
#elif defined(LINUS_LINUX)
    mProblemInfo.mSolutionFileName = "2/plus.cpp";
#endif 
#endif

    sys::Compiler compiler;

    auto codePath = MAEDIA + mProblemInfo.mSolutionFileName.get();
    auto desirableExecutablePath = SOLUTION_PATH + "-" +
        std::to_string(mProblemInfo.mID);
	cor::CodeInfo codeInfo(codePath.c_str(), );
    compiler.prepareExecutableCommand
    (
        codePath,
        desirableExecutablePath,
        mSolutionParameters
    );

    codePath = MAEDIA + mProblemInfo.mCheckerFileName;
    desirableExecutablePath = CHECKER_PATH + "-" +
        std::to_string(mProblemInfo.mID);

    compiler.prepareExecutableCommand
    (
        codePath,
        desirableExecutablePath,
        mCheckerParameters
    );
    
    check();
}

void
Core::prepareForTesting() noexcept
{
    auto partInfo = mDBQ.getParticipantInfo();
    auto checkInfo = mDBQ.getCheckInfo();

    CodeInfo codeInfo(std::move(partInfo.fileName), "part");
    sys::Compiler comp;
    auto cmd = comp.getExecutableCommand(codeInfo);
}

bool
Core::resultEvoluation(int aCheckNum)
{
    START_LOG_BLOCK("Result_evoluation");
    bool result = false;

    mProblemInfoLock.lock();
    if (mChecksInfo[aCheckNum].mResult.substr(0, 2) != "ok")
    {
        mProblemInfo.mResult = mChecksInfo[aCheckNum].mResult;
        START_LOG_BLOCK("Result_not_okay");
        END_LOG_BLOCK("Today_result_is:", mProblemInfo.mResult);
    }
    else if (mChecksInfo[aCheckNum].mUsedTime > mProblemInfo.mTimeLimit * 1000000)
    {
        mProblemInfo.mResult = "tle";
        START_LOG_BLOCK("Result_is_TLE");
        WRITE_LOG("wanted:", mProblemInfo.mTimeLimit);
        END_LOG_BLOCK("received:", mChecksInfo[aCheckNum].mUsedTime);
    }
    else if (mChecksInfo[aCheckNum].mUsedMemory > mProblemInfo.mMemoryLimit)
    {
        mProblemInfo.mResult = "mle";
        START_LOG_BLOCK("Result_is_MLE");
        WRITE_LOG("wanted:", mProblemInfo.mMemoryLimit);
        END_LOG_BLOCK("received:", mChecksInfo[aCheckNum].mUsedMemory);
    }
    else
    {
        result = true;
        if (mProblemInfo.mUsedTime < mChecksInfo[aCheckNum].mUsedTime)
            mProblemInfo.mUsedTime = mChecksInfo[aCheckNum].mUsedTime;
        if (mProblemInfo.mUsedMemory < mChecksInfo[aCheckNum].mUsedMemory)
            mProblemInfo.mUsedMemory = mChecksInfo[aCheckNum].mUsedMemory;
        START_LOG_BLOCK("ok_test_passed");
        END_LOG_BLOCK();
    }
    mProblemInfoLock.unlock();

    END_LOG_BLOCK();
    return result;
}

void
Core::check
(
    //std::string& aSolutionName,
    //std::string& aCheckerName
    //const std::vector<char*>& aSolutionParameters,
    //const std::vector<char*>& aCheckerParameters
)
{
    START_LOG_BLOCK("Checking_participant_code");

    mDBQ.prepareTestsStatement(mProblemInfo);
   //pipesTesting(0);

    for (bool isStillTesting = true; isStillTesting;)
    {
        isStillTesting = false;
        for (uint_32 i = 0; i < mChecks.size(); ++i)
        {
            bool needEvaluateResult = false;

            mChecksMutexs[i].lock();
            isStillTesting |= !mChecksInfo[i].mIsFinishedTesting;
            if (mChecksInfo[i].mIsFinishedTesting && mChecks[i] != NULL)
            {
                mChecks[i]->join();
                ++mFinishedTest;
                isStillTesting = true;
                delete mChecks[i];

                isStillTesting = resultEvoluation(i);

                if (!mProblemInfo.mTestsAreOver && isStillTesting) {
                    mChecksInfo[i].mIsFinishedTesting = false;
                    mChecks[i] = new std::thread(&Core::pipesTesting, this,
                        i//, std::ref(aSolutionName), std::ref(aCheckerName)
                        //i, aSolutionName, aCheckerName
                    );
                }
                else
                {
                    mChecks[i] = NULL;
                }
            }

            mChecksMutexs[i].unlock();

        }
    }

    mProblemInfo.remakeResultForPasha();
    mDBQ.writeResult(mProblemInfo.mID, mProblemInfo.mResult, 
        mProblemInfo.mUsedTime, mProblemInfo.mUsedMemory);

    WRITE_LOG("Final_result:", mProblemInfo.mResult);
    WRITE_LOG("Final_time:", mProblemInfo.mUsedTime);
    END_LOG_BLOCK("Final_memory:", mProblemInfo.mUsedMemory);
}

void
Core::pipesTesting
(
    int             aThreadNum
    //const std::vector<char*>& aSolutionParameters,
    //const std::vector<char*>& aCheckerParameters
    //std::string&    aSolutionName,
    //std::string&    aCheckerName
    //std::string aSolutionName,
    //std::string aCheckerName
)
{
    START_LOG_BLOCK("Checking_test#", aThreadNum + mFinishedTest);
//if (aThreadNum == 100)
//{
//    int yy = 0;
//    ++yy;
//std::cout << yy;
//
//}
    TestLibMessage TLM;
    mGetTestLock.lock();
    if (!mProblemInfo.mTestsAreOver)
    {
        mDBQ.getNextTest(mProblemInfo, TLM);
    }
    if (mProblemInfo.mTestsAreOver)
    {
        mChecksMutexs[aThreadNum].lock();
        mChecksInfo[aThreadNum].mIsFinishedTesting = true;
        mChecksInfo[aThreadNum].mResult = "ok";
        WRITE_LOG("Tests_are_over");
        mChecksMutexs[aThreadNum].unlock();
        mGetTestLock.unlock();
        END_LOG_BLOCK();
        return;
    }
    mGetTestLock.unlock();

    TLM.makeTestSizes();

    //MyProcess code(mSolutionParameters, mProblemInfo.mTimeLimit, mProblemInfo.mMemoryLimit);
    proc::PipeProcess code(mSolutionParameters, mProblemInfo.mTimeLimit, mProblemInfo.mMemoryLimit);

    code.writePipe(TLM.mTest);
    //std::pair<uint_64, uint_64> cur = code.runWithLimits(mProblemInfo.mTimeLimit, mProblemInfo.mMemoryLimit);
    std::pair<uint_64, uint_64> cur = code.runWithLimits();
    if (cur.first == KILLING_PROCESS_TIME_VALUE && 
        cur.second == KILLING_PROCESS_MEMORY_VALUE)
    {
        mChecksInfo[aThreadNum].mResult = "tle";
        mChecksMutexs[aThreadNum].lock();
        mChecksInfo[aThreadNum].mIsFinishedTesting = true;
        mChecksMutexs[aThreadNum].unlock();
        END_LOG_BLOCK();
        return;
    }
    code.readPipe(TLM.mOutput);

    mChecksInfo[aThreadNum].mUsedTime = cur.first;
    mChecksInfo[aThreadNum].mUsedMemory = cur.second;

    //MyProcess checker(mCheckerParameters);
    proc::PipeProcess checker(mCheckerParameters);

//    TLM.mAnswer.pop_back();
//    TLM.mAnswer.pop_back();

    TLM.makeOutputSizes();
    TLM.makeAnswerSizes();

    checker.writePipe(TLM.mTestSize,    proc::PipeProcess::PypeType::NO_ZERO);
    checker.writePipe(TLM.mTest,        proc::PipeProcess::PypeType::NO_ZERO);

    checker.writePipe(TLM.mOutputSize,  proc::PipeProcess::PypeType::NO_ZERO);
    checker.writePipe(TLM.mOutput,      proc::PipeProcess::PypeType::NO_ZERO);

    checker.writePipe(TLM.mAnswerSize,  proc::PipeProcess::PypeType::NO_ZERO);
    checker.writePipe(TLM.mAnswer,      proc::PipeProcess::PypeType::NO_ZERO);

    START_LOG_BLOCK("Test:", TLM.mTest);
    WRITE_LOG("Output:", TLM.mOutput);
    END_LOG_BLOCK("Answer:", TLM.mAnswer);
    END_LOG_BLOCK();

    checker.run();
    checker.readPipe(mChecksInfo[aThreadNum].mResult);

    mChecksMutexs[aThreadNum].lock();
    mChecksInfo[aThreadNum].mIsFinishedTesting = true;
    mChecksMutexs[aThreadNum].unlock();
}