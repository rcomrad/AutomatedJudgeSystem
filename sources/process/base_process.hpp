#ifndef BASE_PROCESS_H
#define BASE_PROCESS_H

//--------------------------------------------------------------------------------
//				CHILD PROCESS CREATION IMPLIMENTATION 
//--------------------------------------------------------------------------------

#include <string>
#include <vector>
#include <iostream>
#include <optional>

#include "domain/type.hpp"
#include "domain/string.hpp"
#include "domain/error_message.hpp"
#include "domain/path.hpp"
#include "domain/pair.hpp"
#include "domain/class_member_functions.hpp"

//--------------------------------------------------------------------------------
// PROCESS CREATION CONSTANTS 
//--------------------------------------------------------------------------------

#define MAX_TIME_LIMIT 5000
#define MAX_MEMORY_LIMIT 5000

//--------------------------------------------------------------------------------

namespace proc
{
	class BaseProcess
	{
	public:
		enum class PypeType { ZERO = 0, NO_ZERO = 1 };

		/*
		\brief Base process constructor that initialize time and memory 
		usage limits for child process.
		(all child process have max time and memory usage limits
		that defined in MAX_TIME_LIMIT and MAX_MEMORY_LIMIT)
		\param aParameters Child process parameters for execution.
		\param aTimeLimit Child process maximum time usage.
		\param aMemoryLimit Child process maximum memory usage.
		*/
		BaseProcess() noexcept;
		// virtual ~Process() = default;
        // Process(const Process& other) noexcept = default;
        // Process(Process&& other) noexcept = default;

        BaseProcess(BaseProcess&& other) noexcept = default;
        BaseProcess(const BaseProcess& other) noexcept = default;

        BaseProcess& operator=(BaseProcess&& other) noexcept = default;
        BaseProcess& operator=(const BaseProcess& other) noexcept = default;

        ~BaseProcess() = default;


        virtual void setComand(const dom::StringTable& aParameters) noexcept = 0;
        /*
		\brief Create a child process with the specified parameters.
		\param aParameters Child process parameters for execution.
		*/
		virtual void create() noexcept = 0;

		/*
		\brief Sets time and memory usage limits.
		\param aTimeLimit Child process maximum time usage.
		\param aMemoryLimit Child process maximum memory usage.
		*/
		void setLimits(uint_64 aTimeLimit, uint_64 aMemoryLimit) noexcept;

		/*
		\brief Executing the child process without time and memory 
		usage evaluation.
		\return Returns true if the process is completed successfully.
		*/
		virtual bool run() noexcept = 0;

		/*
		\brief Executing the child process with time and memory 
		usage evaluation.
		\return Returns the time and memory used by the process.
		*/
		virtual std::optional<dom::Pair<uint_64>> runWithLimits() noexcept = 0;

        virtual void write(const std::string& aMessage) noexcept = 0;
        virtual void read(std::string& aMessage) noexcept = 0;

	protected:
		/*
		\brief Redirecting input and output streams for a child process.
		*/
		virtual void IORedirection() = 0;

		/*
		\brief Closes the input and output handler for the child process.
		(finishing of communication with the child process)
		*/

		uint_64 mTimeLimit;
		uint_64 mMemoryLimit;
	};

}

//--------------------------------------------------------------------------------

#endif // !BASE_PROCESS_H