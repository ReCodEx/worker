#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/helpers/topological_sort.h"

class test_task : public task_base
{
public:
	test_task() = delete;
	test_task(size_t id, std::shared_ptr<task_metadata> task_meta = std::make_shared<task_metadata>())
		: task_base(id, task_meta)
	{
	}
	virtual ~test_task()
	{
	}
	virtual std::shared_ptr<task_results> run()
	{
		return nullptr;
	}
};

using namespace std;


TEST(topological_sort_test, top_sort_1)
{
	// initialization
	size_t id = 0;
	vector<shared_ptr<task_base>> result;
	vector<shared_ptr<task_base>> expected;


	/*
	 * TASK TREE:
	 *
	 *    A
	 *   / \
	 *  B   C
	 *
	 * priority: A = 1; B = 2; C = 3
	 *
	 * expected = A, C, B
	 */
	shared_ptr<task_base> A = make_shared<test_task>(id++, std::make_shared<task_metadata>("A", 1));
	shared_ptr<task_base> B = make_shared<test_task>(id++, std::make_shared<task_metadata>("B", 2));
	shared_ptr<task_base> C = make_shared<test_task>(id++, std::make_shared<task_metadata>("C", 3));
	A->add_children(B);
	B->add_parent(A);
	A->add_children(C);
	C->add_parent(A);
	expected = {A, C, B};

	// sort itself
	helpers::topological_sort(A, result);
	// and check it
	ASSERT_EQ(result, expected);


	/*
	 * TASK TREE:
	 *
	 *    A
	 *   / \
	 *  B   C
	 *       \
	 *        D
	 *
	 * priority: A = 1; B = 2; C = 3, D = 4
	 *
	 * expected = A, C, D, B
	 */
	shared_ptr<task_base> D = make_shared<test_task>(id++, std::make_shared<task_metadata>("D", 4));
	C->add_children(D);
	D->add_parent(C);
	expected = {A, C, D, B};

	// sort itself
	helpers::topological_sort(A, result);
	// and check it
	ASSERT_EQ(result, expected);
}

TEST(topological_sort_test, top_sort_2)
{
	// initialization
	size_t id = 0;
	map<string, size_t> eff_ind;
	vector<shared_ptr<task_base>> result;
	vector<shared_ptr<task_base>> expected;


	/*
	 * TASK TREE:
	 *
	 *   A
	 *    \
	 *     B
	 *      \
	 *       C
	 *
	 * priority: A = 1; B = 3; C = 2
	 *
	 * expected = A, B, C
	 */
	shared_ptr<task_base> A = make_shared<test_task>(id++, std::make_shared<task_metadata>("A", 1));
	shared_ptr<task_base> B = make_shared<test_task>(id++, std::make_shared<task_metadata>("B", 2));
	shared_ptr<task_base> C = make_shared<test_task>(id++, std::make_shared<task_metadata>("C", 3));
	A->add_children(B);
	B->add_parent(A);
	B->add_children(C);
	C->add_parent(B);
	expected = {A, B, C};

	// sort itself
	helpers::topological_sort(A, result);
	// and check it
	ASSERT_EQ(result, expected);


	/*
	 * TASK TREE:
	 *
	 *   A
	 *    \
	 *     B
	 *      \
	 *       C
	 *       /\\
	 *      F E D
	 *
	 * priority: A = 1; B = 3; C = 2, D = 4, E = 4, F = 4
	 *
	 * expected = A, B, C, D, E, F
	 */
	shared_ptr<task_base> D = make_shared<test_task>(id++, std::make_shared<task_metadata>("D", 4));
	shared_ptr<task_base> E = make_shared<test_task>(id++, std::make_shared<task_metadata>("E", 4));
	shared_ptr<task_base> F = make_shared<test_task>(id++, std::make_shared<task_metadata>("F", 4));
	C->add_children(D);
	D->add_parent(C);
	C->add_children(E);
	E->add_parent(C);
	C->add_children(F);
	F->add_parent(C);
	expected = {A, B, C, D, E, F};

	// sort itself
	helpers::topological_sort(A, result);
	// and check it
	ASSERT_EQ(result, expected);
}

TEST(topological_sort_test, top_sort_3)
{
	// initialization
	size_t id = 0;
	vector<shared_ptr<task_base>> result;
	vector<shared_ptr<task_base>> expected;


	/*
	 * TASK TREE:
	 *
	 *     __________ A _________
	 *    ///    ///     \\\   \\\
	 *   D B C  G E F   J H I  M K L
	 *
	 * priority: A = 1, B = 2, C = 2, D = 2,
	 *           H = 3, I = 3, J = 3,
	 *           E = 4, F = 4, G = 4,
	 *           K = 5, L = 5, M = 5
	 *
	 * expected = A,
	 *            K, L, M,
	 *            E, F, G,
	 *            H, I, J,
	 *            B, C, D
	 */
	shared_ptr<task_base> A = make_shared<test_task>(id++, std::make_shared<task_metadata>("A", 1));
	shared_ptr<task_base> B = make_shared<test_task>(id++, std::make_shared<task_metadata>("B", 2));
	shared_ptr<task_base> C = make_shared<test_task>(id++, std::make_shared<task_metadata>("C", 2));
	shared_ptr<task_base> D = make_shared<test_task>(id++, std::make_shared<task_metadata>("D", 2));
	shared_ptr<task_base> E = make_shared<test_task>(id++, std::make_shared<task_metadata>("E", 4));
	shared_ptr<task_base> F = make_shared<test_task>(id++, std::make_shared<task_metadata>("F", 4));
	shared_ptr<task_base> G = make_shared<test_task>(id++, std::make_shared<task_metadata>("G", 4));
	shared_ptr<task_base> H = make_shared<test_task>(id++, std::make_shared<task_metadata>("H", 3));
	shared_ptr<task_base> I = make_shared<test_task>(id++, std::make_shared<task_metadata>("I", 3));
	shared_ptr<task_base> J = make_shared<test_task>(id++, std::make_shared<task_metadata>("J", 3));
	shared_ptr<task_base> K = make_shared<test_task>(id++, std::make_shared<task_metadata>("K", 5));
	shared_ptr<task_base> L = make_shared<test_task>(id++, std::make_shared<task_metadata>("L", 5));
	shared_ptr<task_base> M = make_shared<test_task>(id++, std::make_shared<task_metadata>("M", 5));
	A->add_children(B);
	A->add_children(C);
	A->add_children(D);
	A->add_children(E);
	A->add_children(F);
	A->add_children(G);
	A->add_children(H);
	A->add_children(I);
	A->add_children(J);
	A->add_children(K);
	A->add_children(L);
	A->add_children(M);
	B->add_parent(A);
	C->add_parent(A);
	D->add_parent(A);
	E->add_parent(A);
	F->add_parent(A);
	G->add_parent(A);
	H->add_parent(A);
	I->add_parent(A);
	J->add_parent(A);
	K->add_parent(A);
	L->add_parent(A);
	M->add_parent(A);
	expected = {A, K, L, M, E, F, G, H, I, J, B, C, D};

	// sort itself
	helpers::topological_sort(A, result);
	// and check it
	ASSERT_EQ(result, expected);
}

TEST(topological_sort_test, top_sort_4)
{
	// initialization
	size_t id = 0;
	vector<shared_ptr<task_base>> result;
	vector<shared_ptr<task_base>> expected;


	/*
	 * TASK TREE:
	 *
	 *      A
	 *     / \
	 *    B   D _
	 *     \ /  \\
	 *      C   E F
	 *     /
	 *    G
	 *
	 * priority: A = 1, B = 4, C = 6, D = 2, E = 3, F = 5, G = 7
	 *
	 * expected = A, B, D, C, G, F, E
	 */
	shared_ptr<task_base> A = make_shared<test_task>(id++, std::make_shared<task_metadata>("A", 1));
	shared_ptr<task_base> B = make_shared<test_task>(id++, std::make_shared<task_metadata>("B", 4));
	shared_ptr<task_base> C = make_shared<test_task>(id++, std::make_shared<task_metadata>("C", 6));
	shared_ptr<task_base> D = make_shared<test_task>(id++, std::make_shared<task_metadata>("D", 2));
	shared_ptr<task_base> E = make_shared<test_task>(id++, std::make_shared<task_metadata>("E", 3));
	shared_ptr<task_base> F = make_shared<test_task>(id++, std::make_shared<task_metadata>("F", 5));
	shared_ptr<task_base> G = make_shared<test_task>(id++, std::make_shared<task_metadata>("G", 7));
	A->add_children(B);
	B->add_parent(A);
	B->add_children(C);
	C->add_parent(B);
	D->add_children(C);
	C->add_parent(D);
	A->add_children(D);
	D->add_parent(A);
	D->add_children(E);
	E->add_parent(D);
	D->add_children(F);
	F->add_parent(D);
	C->add_children(G);
	G->add_parent(C);
	expected = {A, B, D, C, G, F, E};

	// sort itself
	helpers::topological_sort(A, result);
	// and check it
	ASSERT_EQ(result, expected);
}

TEST(topological_sort_test, top_sort_5)
{
	// initialization
	size_t id = 0;
	vector<shared_ptr<task_base>> result;
	vector<shared_ptr<task_base>> expected_result;


	/*
	 * TASK TREE:
	 *
	 *           root
	 *            |
	 *   -------mkdir-------
	 *   |      / \ \      |
	 *   |     /  |  \     |
	 *   |   GCC  |   \    |
	 *   |     \  |   |    |
	 *   |      \ |   |    |
	 *  input---EXEC  |  expected
	 *             \  |   /
	 *              \ |  /
	 *              judge
	 *
	 *
	 * priority: root = 1, mkdir = 1, GCC = 100, EXEC = 90, judge = 80, input = 1, expected = 1
	 *
	 * expected = root, mkdir, GCC, input, EXEC, expected, judge
	 */
	shared_ptr<task_base> root = make_shared<test_task>(id++, std::make_shared<task_metadata>("root", 1));
	shared_ptr<task_base> mkdir = make_shared<test_task>(id++, std::make_shared<task_metadata>("mkdir", 1));
	shared_ptr<task_base> GCC = make_shared<test_task>(id++, std::make_shared<task_metadata>("GCC", 100));
	shared_ptr<task_base> input = make_shared<test_task>(id++, std::make_shared<task_metadata>("input", 1));
	shared_ptr<task_base> EXEC = make_shared<test_task>(id++, std::make_shared<task_metadata>("EXEC", 90));
	shared_ptr<task_base> expected = make_shared<test_task>(id++, std::make_shared<task_metadata>("expected", 1));
	shared_ptr<task_base> judge = make_shared<test_task>(id++, std::make_shared<task_metadata>("judge", 80));
	root->add_children(mkdir);
	mkdir->add_parent(root);
	mkdir->add_children(input);
	mkdir->add_children(GCC);
	mkdir->add_children(EXEC);
	mkdir->add_children(judge);
	mkdir->add_children(expected);
	GCC->add_parent(mkdir);
	GCC->add_children(EXEC);
	input->add_parent(mkdir);
	input->add_children(EXEC);
	EXEC->add_parent(mkdir);
	EXEC->add_parent(GCC);
	EXEC->add_parent(input);
	EXEC->add_children(judge);
	expected->add_parent(mkdir);
	expected->add_children(judge);
	judge->add_parent(mkdir);
	judge->add_parent(EXEC);
	judge->add_parent(expected);
	expected_result = {root, mkdir, GCC, input, EXEC, expected, judge};

	// sort itself
	helpers::topological_sort(root, result);
	// and check it
	ASSERT_EQ(result, expected_result);
}

TEST(topological_sort_test, top_sort_6)
{
	// initialization
	size_t id = 0;
	vector<shared_ptr<task_base>> result;
	vector<shared_ptr<task_base>> expected_result;


	/*
	 * TASK TREE:
	 * note: different auto-increment order of elements than in top_sort_5
	 *
	 *           root
	 *            |
	 *   -------mkdir-------
	 *   |      / \ \      |
	 *   |     /  |  \     |
	 *   |   GCC  |   \    |
	 *   |     \  |   |    |
	 *   |      \ |   |    |
	 *  input---EXEC  |  expected
	 *             \  |   /
	 *              \ |  /
	 *              judge
	 *
	 *
	 * priority: root = 1, mkdir = 1, GCC = 100, EXEC = 90, judge = 80, input = 1, expected = 1
	 *
	 * expected = root, mkdir, GCC, input, EXEC, expected, judge
	 */
	shared_ptr<task_base> root = make_shared<test_task>(id++, std::make_shared<task_metadata>("root", 1));
	shared_ptr<task_base> mkdir = make_shared<test_task>(id++, std::make_shared<task_metadata>("mkdir", 1));
	shared_ptr<task_base> expected = make_shared<test_task>(id++, std::make_shared<task_metadata>("expected", 1));
	shared_ptr<task_base> input = make_shared<test_task>(id++, std::make_shared<task_metadata>("input", 1));
	shared_ptr<task_base> GCC = make_shared<test_task>(id++, std::make_shared<task_metadata>("GCC", 100));
	shared_ptr<task_base> EXEC = make_shared<test_task>(id++, std::make_shared<task_metadata>("EXEC", 90));
	shared_ptr<task_base> judge = make_shared<test_task>(id++, std::make_shared<task_metadata>("judge", 80));
	root->add_children(mkdir);
	mkdir->add_parent(root);
	mkdir->add_children(input);
	mkdir->add_children(GCC);
	mkdir->add_children(EXEC);
	mkdir->add_children(judge);
	mkdir->add_children(expected);
	GCC->add_parent(mkdir);
	GCC->add_children(EXEC);
	input->add_parent(mkdir);
	input->add_children(EXEC);
	EXEC->add_parent(mkdir);
	EXEC->add_parent(GCC);
	EXEC->add_parent(input);
	EXEC->add_children(judge);
	expected->add_parent(mkdir);
	expected->add_children(judge);
	judge->add_parent(mkdir);
	judge->add_parent(EXEC);
	judge->add_parent(expected);
	expected_result = {root, mkdir, GCC, input, EXEC, expected, judge};

	// sort itself
	helpers::topological_sort(root, result);
	// and check it
	ASSERT_EQ(result, expected_result);
}

TEST(topological_sort_test, top_sort_cycle_1)
{
	// initialization
	size_t id = 0;
	vector<shared_ptr<task_base>> result;


	/*
	 * TASK TREE:
	 *
	 *          A
	 *         / \
	 *   ---> B   D _
	 *   |     \ /  \\
	 *   |      C   E F
	 *   |     /
	 *   ---- G
	 *
	 * priority: A = 1, B = 4, C = 6, D = 2, E = 3, F = 5, G = 7
	 *
	 * expected = throw job_exception("Cycle detected")
	 */
	shared_ptr<task_base> A = make_shared<test_task>(id++, std::make_shared<task_metadata>("A", 1));
	shared_ptr<task_base> B = make_shared<test_task>(id++, std::make_shared<task_metadata>("B", 4));
	shared_ptr<task_base> C = make_shared<test_task>(id++, std::make_shared<task_metadata>("C", 6));
	shared_ptr<task_base> D = make_shared<test_task>(id++, std::make_shared<task_metadata>("D", 2));
	shared_ptr<task_base> E = make_shared<test_task>(id++, std::make_shared<task_metadata>("E", 3));
	shared_ptr<task_base> F = make_shared<test_task>(id++, std::make_shared<task_metadata>("F", 5));
	shared_ptr<task_base> G = make_shared<test_task>(id++, std::make_shared<task_metadata>("G", 7));
	A->add_children(B);
	B->add_parent(A);
	B->add_children(C);
	C->add_parent(B);
	D->add_children(C);
	C->add_parent(D);
	A->add_children(D);
	D->add_parent(A);
	D->add_children(E);
	E->add_parent(D);
	D->add_children(F);
	F->add_parent(D);
	C->add_children(G);
	G->add_parent(C);
	G->add_children(B);
	B->add_parent(G);

	EXPECT_THROW(helpers::topological_sort(A, result), helpers::top_sort_exception);
}

TEST(topological_sort_test, top_sort_cycle_2)
{
	// initialization
	size_t id = 0;
	vector<shared_ptr<task_base>> result;


	/*
	 * TASK TREE:
	 *
	 *          A -> B
	 *          ^    |
	 *          |    |
	 *          D <- C
	 *
	 * priority: A = 1, B = 2, C = 3, D = 4
	 *
	 * expected = throw job_exception("Cycle detected")
	 */
	shared_ptr<task_base> A = make_shared<test_task>(id++, std::make_shared<task_metadata>("A", 1));
	shared_ptr<task_base> B = make_shared<test_task>(id++, std::make_shared<task_metadata>("B", 2));
	shared_ptr<task_base> C = make_shared<test_task>(id++, std::make_shared<task_metadata>("C", 3));
	shared_ptr<task_base> D = make_shared<test_task>(id++, std::make_shared<task_metadata>("D", 4));
	A->add_children(B);
	B->add_parent(A);
	B->add_children(C);
	C->add_parent(B);
	C->add_children(D);
	D->add_parent(C);
	D->add_children(A);
	A->add_parent(D);

	EXPECT_THROW(helpers::topological_sort(A, result), helpers::top_sort_exception);
}
