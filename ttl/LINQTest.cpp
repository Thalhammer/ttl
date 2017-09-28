#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "linq.h"


using thalhammer::linq;

TEST(LINQTest, IteratorMembers) {
	std::vector<std::string> data{ "Hello", "World", "how", "are", "you", "?" };

	auto query = linq(data);
	ASSERT_FALSE(query.is_end());
	ASSERT_EQ("Hello", query.element());
	ASSERT_TRUE(query.next());
	ASSERT_FALSE(query.is_end());
	ASSERT_EQ("World", query.element());
	ASSERT_TRUE(query.next());
	ASSERT_FALSE(query.is_end());
	ASSERT_EQ("how", query.element());
	ASSERT_TRUE(query.next());
	ASSERT_FALSE(query.is_end());
	ASSERT_EQ("are", query.element());
	ASSERT_TRUE(query.next());
	ASSERT_FALSE(query.is_end());
	ASSERT_EQ("you", query.element());
	ASSERT_TRUE(query.next());
	ASSERT_FALSE(query.is_end());
	ASSERT_EQ("?", query.element());
	ASSERT_FALSE(query.next());
	ASSERT_TRUE(query.is_end());
}

TEST(LINQTest, Where) {
	std::vector<std::string> data{ "Hello", "World", "how", "are", "you", "?" };

	auto query = linq(data).where([](const std::string& data) { return data.size() == 3; }).to_vector();
	ASSERT_EQ(3, query.size());
	ASSERT_EQ("how", query[0]);
	ASSERT_EQ("are", query[1]);
	ASSERT_EQ("you", query[2]);
}

TEST(LINQTest, Single) {
	std::vector<std::string> data{ "Hello", "World", "how", "are", "you", "?" };

	ASSERT_THROW({
		linq(data).single();
	}, std::logic_error);
	ASSERT_THROW({
		linq(data).single_or_default("");
	}, std::logic_error);

	data.clear();

	ASSERT_THROW({
		linq(data).single();
	}, std::logic_error);

	ASSERT_NO_THROW({
		linq(data).single_or_default("");
	});

	data.push_back("Single");

	std::string res;
	ASSERT_NO_THROW({
		res = linq(data).single();
	});
	ASSERT_EQ("Single", res);

	ASSERT_NO_THROW({
		res = linq(data).single_or_default("");
	});
	ASSERT_EQ("Single", res);
}

TEST(LINQTest, First) {
	std::vector<std::string> data{ "Hello", "World", "how", "are", "you", "?" };
	std::string res;
	ASSERT_NO_THROW({
		res = linq(data).first();
	});
	ASSERT_EQ("Hello", res);

	ASSERT_NO_THROW({
		res = linq(data).first_or_default("");
	});
	ASSERT_EQ("Hello", res);

	data.clear();

	ASSERT_THROW({
		linq(data).first();
	}, std::logic_error);

	ASSERT_NO_THROW({
		res = linq(data).first_or_default("");
	});
	ASSERT_EQ("", res);
}

TEST(LINQTest, Last) {
	std::vector<std::string> data{ "Hello", "World", "how", "are", "you", "?" };
	std::string res;
	ASSERT_NO_THROW({
		res = linq(data).last();
	});
	ASSERT_EQ("?", res);

	ASSERT_NO_THROW({
		res = linq(data).last_or_default("");
	});
	ASSERT_EQ("?", res);

	data.clear();

	ASSERT_THROW({
		linq(data).last();
	}, std::logic_error);
	ASSERT_NO_THROW({
		res = linq(data).last_or_default("");
	});
	ASSERT_EQ("", res);
}

TEST(LINQTest, ElementAt) {
	std::vector<std::string> data{ "Hello", "World", "how", "are", "you", "?" };
	std::string res;
	ASSERT_NO_THROW({
		res = linq(data).element_at(3);
	});
	ASSERT_EQ("are", res);

	ASSERT_NO_THROW({
		res = linq(data).element_at_or_default(3,"");
	});
	ASSERT_EQ("are", res);

	data.clear();

	ASSERT_THROW({
		linq(data).element_at(3);
	}, std::logic_error);
	ASSERT_NO_THROW({
		res = linq(data).element_at_or_default(3,"");
	});
	ASSERT_EQ("", res);
}

TEST(LINQTest, Count) {
	std::vector<std::string> data{ "Hello", "World", "how", "are", "you", "?" };

	ASSERT_EQ(6, linq(data).count());
}

TEST(LINQTest, Sum) {
	std::vector<int> data{ 10, 30, 20 };

	ASSERT_EQ(60, linq(data).sum());
}

TEST(LINQTest, Avg) {
	std::vector<int> data{ 10, 30, 20 };

	ASSERT_EQ(20, linq(data).avg());
}

TEST(LINQTest, Max) {
	std::vector<int> data{ 10, 30, 20 };

	ASSERT_EQ(30, linq(data).max());
}

TEST(LINQTest, Min) {
	std::vector<int> data{ 30, 10, 20 };

	ASSERT_EQ(10, linq(data).min());
}

TEST(LINQTest, Select) {
	std::vector<int> data{ 30, 10, 20 };

	auto res = linq(data).select([](const int& i) { return std::to_string(i); }).to_vector();
	ASSERT_EQ(3, res.size());
	ASSERT_EQ("30", res[0]);
	ASSERT_EQ("10", res[1]);
	ASSERT_EQ("20", res[2]);
}

TEST(LINQTest, GroupBy) {
	struct Employee {
		std::string name;
		uint64_t department;
	};

	std::vector<Employee> data{ { "Max", 1 },{ "Muster", 2 },{ "Mann", 1 } };

	auto res = linq(data)
		.groupby([](const Employee& e) { return e.department; })
		.to_vector();

	ASSERT_EQ(2, res.size());
	ASSERT_EQ(1, res[0].first);
	ASSERT_EQ(2, res[0].second.size());
	ASSERT_EQ(2, res[1].first);
	ASSERT_EQ(1, res[1].second.size());
	ASSERT_EQ(1, res[0].second[0].department);
	ASSERT_EQ(1, res[0].second[1].department);
	ASSERT_EQ(2, res[1].second[0].department);
	ASSERT_EQ("Max", res[0].second[0].name);
	ASSERT_EQ("Mann", res[0].second[1].name);
	ASSERT_EQ("Muster", res[1].second[0].name);
}

TEST(LINQTest, Order) {
	struct Employee {
		std::string name;
		uint64_t department;
	};

	std::vector<Employee> data{ { "B", 1 },{ "C", 2 },{ "A", 1 } };

	auto res = linq(data)
		.orderby([](const Employee& e) { return e.name; })
		.to_vector();

	ASSERT_EQ(3, res.size());
	ASSERT_EQ("A", res[0].name);
	ASSERT_EQ("B", res[1].name);
	ASSERT_EQ("C", res[2].name);

	res = linq(data)
		.orderby_descending([](const Employee& e) { return e.name; })
		.to_vector();

	ASSERT_EQ(3, res.size());
	ASSERT_EQ("C", res[0].name);
	ASSERT_EQ("B", res[1].name);
	ASSERT_EQ("A", res[2].name);
}

TEST(LINQTest, AnonStruct) {
	struct Employee {
		std::string name;
		uint64_t department;
	};

	std::vector<Employee> data{ { "Max", 1 },{ "Muster", 2 },{ "Mann", 1 } };

	auto res = linq(data)
		.select([](const auto& elem) {
		struct temp { std::string test; } t{ elem.name };
		return t;
	})
		.to_vector();

	ASSERT_EQ(3, res.size());
	ASSERT_EQ("Max", res[0].test);
	ASSERT_EQ("Muster", res[1].test);
	ASSERT_EQ("Mann", res[2].test);
}

TEST(LINQTest, Quantifier) {
	std::vector<std::string> data{ "Hello", "World", "how", "are", "you", "?" };

	auto res = linq(data).all([](const std::string& elem) { return !elem.empty(); });
	ASSERT_TRUE(res);
	res = linq(data).all([](const auto& elem) { return elem.size() == 3; });
	ASSERT_FALSE(res);

	res = linq(data).any([](auto& e) { return e.size() == 3; });
	ASSERT_TRUE(res);
	res = linq(data).any([](auto& e) { return e.empty(); });
	ASSERT_FALSE(res);

	ASSERT_TRUE(linq(data).contains("World"));
	ASSERT_FALSE(linq(data).contains("notexistent"));
}
