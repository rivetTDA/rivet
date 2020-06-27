FILES:

1. run_tests.sh - Contains all tests for RIVET Output.

2. correct - Contains expected output of all the tests.

3. latest - Contains output from latest run of run_tests.sh.

4. test_data - Contains all the data files for testing.

5. rivet-test - The module invariants file generated during testing.

USAGE:

> Run tests from the "test/output_test" folder.
> Simply run "./run_tests.sh".
> It will run the tests, compare the outputs and say if tests passed or failed.
> If you have failed tests, it will output the differences.
> Read through to figure out which test(s) failed to help with debugging.
> There are tests that are supposed to generate errors.
> There is no output for these tests in the "correct" file.
> The errors are in the terminal.
> Look at the errors to determine if RIVET correctly catches errors.

MODIFICATION:

> Add tests at the end maintaining test numbers.
> Look at other tests to understand how to write new tests.
> Note that if a test is supposed to generate an error, it is written slightly differently.
> Add comments if test is for a new flag.
> To test with different file(s) or flag(s) you can add a "command" at the top where all the other commands are.
> A "command" is run on the command line, with the arguments that are supplied in the tests.
> If you add a test or modify a test, run_tests.sh will fail.
> Once it fails, confirm that the result from the modified (or added) test is desirable.
> Copy the newly generated latest file over the correct file.