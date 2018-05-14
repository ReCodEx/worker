#include "reader.hpp"
#include "comparator.hpp"

#include <cli/args.hpp>
#include <cli/logger.hpp>
#include <misc/ptr_fix.hpp>

#include <iostream>


template <class READER, class LINE_COMPARATOR>
bool compareOrderedLines(READER &correctReader, READER &resultReader, LINE_COMPARATOR &lineComparator)
{
	return true;
}



int main(int argc, char *argv[])
{
	/*
	 * Arguments
	 */
	bpp::ProgramArguments args(2, 2);
	args.setNamelessCaption(0, "Expected (correct) output file.");
	args.setNamelessCaption(1, "Tested solution output file for verification.");
	try {
		// Reader args
		args.registerArg(bpp::make_unique<bpp::ProgramArguments::ArgBool>(
			"ignore-empty-lines", "Empty lines are ignored completely."));
		args.registerArg(bpp::make_unique<bpp::ProgramArguments::ArgBool>(
			"allow-comments", "Lines starting with '#' are ignored completely."));
		args.registerArg(bpp::make_unique<bpp::ProgramArguments::ArgBool>(
			"ignore-line-ends", "New lines characters are treated as regular whitespace."));
		args.getArg("ignore-empty-lines").conflictsWith("ignore-line-ends");

		// Token comparator args
		args.registerArg(bpp::make_unique<bpp::ProgramArguments::ArgBool>(
			"case-insensitive", "Alphanumeric tokens are compared without case sensitivity."));
		args.registerArg(bpp::make_unique<bpp::ProgramArguments::ArgBool>(
			"numeric", "Tokens which appear to be integers or floats in decimal notation are compared as numbers."));
		args.registerArg(bpp::make_unique<bpp::ProgramArguments::ArgFloat>("float-tolerance",
			"Allowed maximal error for float number comparisons. The error of two numbers is |a-b|/|a+b|.",
			false,
			0.0001,
			0.0,
			1.0));

		// Comparisson strategies
		args.registerArg(bpp::make_unique<bpp::ProgramArguments::ArgBool>(
			"schuffled-tokens", "Tokens on a line may appear in any order."));
		args.registerArg(
			bpp::make_unique<bpp::ProgramArguments::ArgBool>("schuffled-lines", "Lines may appear in any order."));
		args.getArg("schuffled-lines").conflictsWith("ignore-line-ends");

		// Log args
		args.registerArg(
			bpp::make_unique<bpp::ProgramArguments::ArgInt>("log-limit", "Maximal length of the log (in bytes)."));

		// Process the arguments ...
		args.process(argc, argv);
	} catch (bpp::ArgumentException &e) {
		std::cerr << "Error: " << e.what() << std::endl << std::endl;
		args.printUsage(std::cerr);
		return 2;
	}


	try {
		// Initialize logging ...
		bpp::log(bpp::make_unique<bpp::Logger>(std::cerr));
		if (args.getArg("log-limit").isPresent()) {
			bpp::log().restrictSize((std::size_t)args.getArgInt("log-limit").getValue());
		}


		// Open data readers ...
		Reader<> correctReader(args.getArgBool("ignore-empty-lines").getValue(),
			args.getArgBool("allow-comments").getValue(),
			args.getArgBool("ignore-line-ends").getValue());
		Reader<> resultReader(args.getArgBool("ignore-empty-lines").getValue(),
			args.getArgBool("allow-comments").getValue(),
			args.getArgBool("ignore-line-ends").getValue());
		
		correctReader.open(args[0]);
		resultReader.open(args[1]);


		// Initialize comparators
		TokenComparator<> tokenComparator(args.getArgBool("case-insensitive").getValue(),
			args.getArgBool("numeric").getValue(),
			args.getArgFloat("float-tolerance").getValue());

		LineComparator<> lineComparator(tokenComparator, args.getArgBool("schuffled-tokens").getValue());


		// Execute selected comparisson strategy ...
		bool correct = compareOrderedLines(correctReader, resultReader, lineComparator);
		std::cout << (correct ? 1.0 : 0.0) << std::endl;


		// Finalize ...
		bpp::log().flush();

		correctReader.close();
		resultReader.close();

		return correct ? 0 : 1;

	} catch (std::exception &e) {
		std::cout << 0.0 << std::endl;
		std::cerr << "Error: " << e.what() << std::endl << std::endl;
		args.printUsage(std::cerr);
		return 2;
	}

	return 0;
}
