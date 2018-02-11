## C++ LINQ

C#'s Linq is a really great tool and makes a lot of tasks really easy and fast to solve. Because of this,
I wrote a c++ class that aims at providing similar capabilities to C++.

It can be used with anything that provides iterators (most stl containers and a lot of custom
classes) as well as c style arrays and allows writing complex operations in concise and simple way.

Example (Sum all int smaller than 50):
```cpp
int data[] = { 10, 20, 50, 20 };
auto sum = linq(data).where([](int i) { return i < 50; }).sum();
```

Same example using predicates:
```cpp
int data[] = { 10, 20, 50, 20 };
auto sum = linq(data).where(lt(50)).sum();
```

You can also modify elements on the fly:
```cpp
int data[] = { 30, 10, 20 };
// res is of type std::vector<std::string> and contains { "30", "10", "20" }
auto res = linq(data).select([](const int& i) { return std::to_string(i); }).to_vector();
```

There are also a lot of functions that allow sorting, grouping and filtering of queries.

### Tricks
* You can pass a member pointer to select, orderby and groupby.
* You can iterate over the result without converting to a container using for_each member.
  You pass a member function that gets called on each result. Return true to continue or false to break.

### Warnings
There are some things to keep aware of.

#### Storing queries
The following code would be invalid:
```cpp
auto q = linq(data).where(lt(50));
auto result = q.sum();
```
Because linq returns a iterator that is referenced by the iterator returned by where.
However in C++ temporaries are destroyed after the containing expression is evaluated,
which means the whereiterator now references a value that no more exists.
General rule: Do not store unfinished queries. Doing so **will** cause harm.