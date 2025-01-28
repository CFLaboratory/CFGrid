# Style guide

> The key words "**MUST**", "**MUST NOT**", "**REQUIRED**", "**SHALL**", "**SHALL NOT**",
> "**SHOULD**", "**SHOULD NOT**", "**RECOMMENDED**", "**MAY**", and "**OPTIONAL**" in this document
> are to be interpreted as described in [RFC 2119](https://datatracker.ietf.org/doc/html/rfc2119).
> 
> Note that the force of these words is modified by the requirement level of the document in which
> they are used.
> 
> 1. **MUST** This word, or the terms "**REQUIRED**" or "**SHALL**", mean that the definition is an
>    absolute requirement of the specification.
> 2. **MUST NOT** This phrase, or the phrase "**SHALL NOT**", mean that the definition is an absolute
>    prohibition of the specification.
> 3. **SHOULD** This word, or the adjective "**RECOMMENDED**", mean that there may exist valid reasons
>    in particular circumstances to ignore a particular item, but the full implications must be
>    understood and carefully weighed before choosing a different course.
> 4. **SHOULD NOT** This phrase, or the phrase "**NOT RECOMMENDED**" mean that there may exist valid
>    reasons in particular circumstances when the particular behavior is acceptable or even useful,
>    but the full implications should be understood and the case carefully weighed before implementing
>    any behavior described with this label.
> 5. **MAY** This word, or the adjective "**OPTIONAL**", mean that an item is truly optional.
>    One vendor may choose to include the item because a particular marketplace requires it or because
>    the vendor feels that it enhances the product while another vendor may omit the same item.
>    An implementation which does not include a particular option **MUST** be prepared to interoperate
>    with another implementation which does include the option, though perhaps with reduced
>    functionality.
>    In the same vein an implementation which does include a particular option **MUST** be prepared to
>    interoperate with another implementation which does not include the option (except, of course,
>    for the feature the option provides.)
> 6. **Guidance in the use of these Imperatives**
>    Imperatives of the type defined in this memo must be used with care and sparingly.
>    In particular, they **MUST** only be used where it is actually required for interoperation or to
>    limit behavior which has potential for causing harm (e.g., limiting retransmisssions).
>    For example, they must not be used to try to impose a particular method on implementors where the
>    method is not required for interoperability.
> 7. **Security Considerations**
>    These terms are frequently used to specify behavior with security implications.
>    The effects on security of not implementing a **MUST** or **SHOULD**, or doing something the
>    specification says **MUST NOT** or **SHOULD NOT** be done may be very subtle.
>    Document authors should take the time to elaborate the security implications of not following
>    recommendations or requirements as most implementors will not have had the benefit of the
>    experience and discussion that produced the specification.
> 8. **Acknowledgments**
>    The definitions of these terms are an amalgam of definitions taken from a number of RFCs.
>    In addition, suggestions have been incorporated from a number of people including Robert Ullmann,
>    Thomas Narten, Neal McBurnett, and Robert Elz.

The coding style used in `CFGrid` is generally based on the
[https://isocpp.github.io/CppCoreGuidelines/](C++ core guidelines) as applied by the `clang-tidy`
tool.
Code MUST conform to the style enforced by `clang-tidy`, in exceptional circumstances where this is
not possible the specific checks should be disabled with a comment to explain the intention, *e.g.*
```
// We are converting the argv char* array to a vector so we cannot avoid the use of C-style arrays
// here.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
[[nodiscard]] std::vector<std::string> get_argvector(int argc, const char* argv[])
{
  if (argc < 2)
  {
    throw std::runtime_error("CFGrid requires at least one argument - the mesh file to read");
  }

  // Extract arguments, dropping program name (argv[0])
  std::vector<std::string> args(argc - 1);
  for (int i = 1; i < argc; i++)
  {
    // Apparently in C++20 we could use a span to resolve this, but currently we are trying to stick
    // to C++17.
    args[i - 1] = std::string(argv[i]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  }

  return args;
}
```

## Functions and subroutines

Although `C/C++` don't *explicitly* distinguish functions and subroutines `CFGrid` code should be
written with this distinction.

Functions MUST return a value and MUST NOT modify their inputs.
In `CFGrid` functions should be written using the following `C++` features to enforce this:
`[[nodiscard]]` to prevent calling code from ignoring return values, and use of `const` to prevent
modification of parameters.
As an example a `CFGrid` function could look like:
```
[[nodiscard]] int f(const int x)
{
  return 2 * x; // Cannot ignore this
}
```

Subroutines SHOULD NOT return values, instead they should return values through their parameters,
input parameters should still be marked `const`:
```
void g(const int x, int &y)
{
  y = 2 * x;
}
```
An exception to the above is "*MPI-style*" subroutines that return an error/status code but otherwise
return their value(s) via parameters, these should apply the same rules as normal subroutines, with
the addition of `[[nodiscard]]` for the error code:
```
[[nodiscard]] int h(const int x, int & y)
{
  int err = 0;
  
  y = 2 * x;
  if (y > 3)
  {
    err = 1;
  }
  
  return err; // Cannot ignore error code
}
```

## Global variables

Global variables SHOULD be avoided as far as possible, and even more so modifying them should be
limited.
Functions MUST NOT modify global variables - which should be avoided anyway - *i.e.* functions
should be *pure*.

## Formatting

Basic code formatting is employed via `clang-format`, this ensures that all code is formatted
consistently.
In addition to the formatting applied by `clang-format`, the `CFGrid` code follows the convention
that `class`es and `struct`s are written in `CamelCase`; variables, functions and methods are
written in `snake_case`, e.g.
```
SomeClass the_thing;
the_thing.do_something();
```
