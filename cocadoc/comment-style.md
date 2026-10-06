# COCADA documentation comment style

This document defines the one accepted way of writing documentation comments
in COCADA headers. `cocadoc -l` checks these rules:

```
cocadoc -l $(find libcocada*/src -name '*.h' -not -path '*thrdpty*')
```

and reports every deviation as a **warning** (there are no errors: nothing is
mandatory in the strict sense):

```
libcocada/src/core/strbuf.h:70: warning: [DC6] strbuf_new_from_str: unknown command @src (did you mean @p src?)
```

Each rule has an ID (`DCn`), which is quoted in the warnings. Anything
missing from a comment (a brief, a parameter, a return value) is also shown
as *undocumented* in the generated documentation.

---

## 1. Scope

The rules apply to documentation comments (`/** ... */` and `/**< ... */`)
in public headers (`*/src/*.h`), excluding third-party code (`thrdpty/`).
Comments in `.c` files and ordinary `/* */` and `//` comments are free-form.

Names starting with `_` (e.g. `struct _vec`, `_BITMASK`) are private by
convention and need not be documented.

---

## 2. Comment form

### DC1. Block comments

A documentation comment opens with `/**` alone on its line, continues with a
` * ` column, and closes with ` */` alone on its line.

```c
/**
 * @brief Returns the number of elements in the vector.
 */
size_t vec_len(const vec *v);
```

`cocadoc` warns about:

- one-line comments (`/** @brief ... */`);
- text on the opening `/**` line;
- text on the closing `*/` line;
- `**/` closers (also in member docs).

Doc comments inside macro bodies (DC14) have the same form, with each line
ended by the `\` line continuation.

### DC2. Member comments

Struct fields and enum constants are documented with a trailing `/**< ... */`
on the same line, in a single line, with no block commands. The text is the
member's brief.

```c
typedef enum {
	OPT_OPTIONAL = 0, /**< Option may or may not be used */
	OPT_REQUIRED = 1  /**< Option must be used on every call */
} clioptneed;
```

---

## 3. File comment

### DC3. Every header has a file comment

After the license, the include guard and the `#include`s, each header has a
file comment with, in this order:

```c
/**
 * @file vec.h
 * @author Paulo Fonseca
 * @brief Vector, a.k.a. dynamic array.
 *
 * Optional module documentation (Markdown, headings allowed).
 */
```

- `@file` is followed by the header's own file name.
- `@author` names an author (one `@author` per author).
- `@brief` is one sentence describing the module.
- The rest is the module documentation, shown at the top of the module's
  page (section 10).
- If AI was involved in writing the file, the file comment also declares it
  with `@ai` (DC15), after the `@author`s.

### DC15. AI involvement

The file comment declares how much AI was involved in writing the file, with
one `@ai` line per AI used:

```c
/**
 * @file cdlexer.h
 * @author Paulo Fonseca
 * @ai ai-generated, Claude (Anthropic)
 * @brief Lightweight C tokenizer for cocadoc.
 */
```

The level is one of:

| level | meaning |
|---|---|
| `human` | Written by humans. AI was not used, or only like a search engine. |
| `ai-informed` | Written by humans. AI explained, reviewed or suggested approaches, but wrote none of the code. |
| `ai-assisted` | Mostly written by humans. AI wrote parts (fixes, functions, tests) that a human reviewed and integrated. |
| `ai-generated` | Mostly or entirely written by AI, directed by a human: the human set the requirements and design decisions, and reviewed and approved the result. |
| `ai-autonomous` | Written by AI with little or no human direction or review. |

The level is about who wrote the code and who directed and checked it, not
about the share of lines, which cannot be measured reliably. When parts of a
file are at different levels, the level describes the bulk of it. In every
case, the human who commits the file is responsible for it.

After the level comes the AI used, e.g. `Claude (Anthropic)` (not needed for
`human`). `@ai` is only used in file comments: the level applies to the
whole file.

`@ai` is optional. A file without it is `human`, and its documentation shows
no AI involvement note.

The generated documentation shows the AI involvement of each header, with a
link to a page explaining the levels (`ai-levels.md`).

---

## 4. Declaration comments

### DC4. Public declarations are documented

Every function, type (`typedef`, `struct`, `union`, `enum`), macro, global
variable, struct field and enum constant declared in a header is part of the
API and should be documented. `cocadoc` warns about each undocumented one,
except include guards and names starting with `_`.

Public macros (e.g. `MIN` and `MAX` in `mathutil.h`) follow the same rules
as functions: `@brief`, one `@param` per macro argument, and `@return` if the
macro is an expression with a meaningful value. This includes the macros
that generate declarations (DC14). A variadic `...` that the macro does not
use (no `__VA_ARGS__` in its body) is not documented.

**`@hide`** leaves a declaration out of the documentation pages, whatever
else its comment says, e.g. a composite generator (DC14) that is an
implementation detail:

```c
/**
 * @hide
 */
#define DECL_HASHMAP_ALL( TYPE , ...) \
	DECL_HASHMAP_GET(TYPE) \
	DECL_HASHMAP_SET(TYPE)
```

A hidden declaration is not listed in the contents, and references to it
are shown as code, not links. `cocadoc` does not check its comment.

### DC5. Explicit, short brief

Every declaration comment starts with `@brief`, followed by **one sentence**
that ends with a period. Start with a verb in the third person for functions
("Returns ...", "Creates ...") and with a noun phrase for types and macros
("Vector type (opaque).").

The brief ends at the first blank line, so write further details as a
separate paragraph:

```c
/**
 * @brief Transforms a raw byte array into a vector.
 *
 * The buffer @p buf becomes the internal buffer of the vector.
 * To create a vector from a copy of a buffer see #vec_new_from_arr_cpy.
 *
 * @param @move buf The buffer containing the vector data.
 * ...
 */
```

`cocadoc` warns about a missing or empty `@brief`, and about briefs with more
than one sentence. Comments without `@brief` still get one (their first
sentence), with a warning. Member docs (DC2) need no `@brief`.

---

## 5. Block commands

### DC6. Allowed commands

Only these block commands are used:

| command | use | notes |
|---|---|---|
| `@file name` | file comment | DC3 |
| `@author name` | file comment | DC3 |
| `@ai level, agent` | file comment | DC15 |
| `@brief text` | every comment | DC5 |
| `@param [@move] name text` | functions and function-like macros | DC8, DC9 |
| `@return [@move] text` | functions returning a value | DC8, DC9 |
| `@see refs` | related symbols or files | DC10 |
| `@warning text` | misuse that leads to bugs | may contain a list |
| `@note text` | remarks | |
| `@deprecated text` | deprecated API | say what to use instead |
| `@hide` | declarations left out of the pages | DC4 |

The only inline command is `@p name`, for parameters of the documented
function (DC11). It saves writing the parameter name in full markup, and
tells a parameter apart from other names.

`cocadoc` warns about every other command, but still understands the common
ones, so that the documentation is complete in the meantime:

| found | write instead |
|---|---|
| `@returns`, `@result` | `@return` |
| `@warn`, `@attention` | `@warning` |
| `@short` | `@brief` |
| `@sa` | `@see` |
| `@remark`, `@remarks` | `@note` |
| `@authors` | `@author` |
| `@par` | `@param` or `@p` (it is usually a typo), or a Markdown heading or **bold** text |
| `@code` ... `@endcode`, `@verbatim` | a ```` ```c ```` fence (DC12) |
| `@c`, `@b`, `@e`, `@em`, `@a`, `@ref`, `@link`, `@n` | Markdown (`` `code` ``, `**bold**`, `*emphasis*`) or `#name` |
| `\brief` etc. | `@brief` etc. |
| an unknown `@word` | usually `@p word` with the space missing, e.g. `@src` |

### DC7. Order of commands

Within a comment, in this order: `@brief`, details (plain paragraphs),
`@param`s, `@return`, `@warning`s, `@note`s, `@deprecated`, `@see`.
(`@file` and `@author` are only used in file comments, before the brief.)

---

## 6. Parameters and return value

### DC8. `@param` and `@return`

```
@param [@move] name description
@return [@move] description
```

- `name` is the exact parameter name, without punctuation (`@param x, ...`
  is wrong) and without direction annotations (`@param[in]`).
- One `@param` per parameter, in declaration order. Variadic parameters are
  documented as `@param ...`.
- Functions returning a value have a `@return`; `void` functions do not.

`cocadoc` compares the `@param`s with the declaration, and warns about
undocumented parameters, `@param`s naming no parameter, and a different
order. It also warns about a missing `@return`, or a `@return` on a `void`
function.

### DC9. Ownership: `@move`

COCADA functions document who owns an object after a call. There are two
cases:

- **borrowed** (the default; no annotation): the function only uses the
  object during the call (or keeps a reference to it, if documented). The
  caller still owns it and remains responsible for destroying it.
- **moved**: ownership moves to the function. After the call, the caller must
  not use, free or destroy the object; the function will destroy it, or it
  becomes part of the function's result (like a vector's buffer).

Moved parameters are marked with `@move` between `@param` and the name:

```c
/**
 * @brief Creates a new non-sc option.
 * @param shortname The short name of the option.
 * @param @move choices The possible values, if @p type is ARG_CHOICE.
 * ...
 */
```

Likewise, `@return @move` means that the caller receives ownership of the
returned object (and must eventually destroy it); a plain `@return` means
the caller gets a borrowed reference:

```c
/**
 * @brief Detaches and returns the internal string, destroying the buffer.
 * @param @move self The string buffer.
 * @return @move The string.
 */
char *strbuf_detach(strbuf *self);
```

`@move` is not restricted to pointer parameters: a value passed by copy may
own heap resources too, e.g. an `ARRAY(T)` (`arrays.h`) is a small struct
whose `arr` member is a heap buffer.

`@move` is the only ownership annotation. The older spellings are still
understood, with a warning:

| found | write instead |
|---|---|
| `(**no transfer**)`, `(no transfer)`, `(*no transfer*)` | nothing (borrowed is the default) |
| `(**move**)`, `(move)`, `(**transfer**)`, `(full transfer)` | `@param @move name` / `@return @move` |
| `@param name @move` | `@param @move name` |

---

## 7. References

### DC10. `@see`

`@see` is followed by a comma-separated list of symbol names (`vec_push`)
and/or file names (`new.h`), and nothing else. Explanations go in the
details.

```c
 * @see vec_new_from_arr_cpy, new.h
```

### DC11. References in text

There is one way to refer to each kind of thing:

| refers to | write | not |
|---|---|---|
| a parameter of the documented function | `@p name` | `@name`, `` `name` `` |
| another function, type, macro or variable | `#name` | `::name`, `name()` |
| a struct field or enum constant | `#type.member` | `type::member` |
| a header | `name.h` | `#name.h`, `::name.h` |

`cocadoc` turns `#name` and `name.h` into links.

Backticks are not references: `` `x + 1` `` is just Markdown for showing a
piece of code in code font (DC12).

---

## 8. Text

### DC12. Markdown, no HTML

Descriptions are Markdown: paragraphs, lists, `*emphasis*`, `**bold**`,
`` `code` ``, tables, and fenced code blocks with a language:

````c
 * ```c
 * vec *v = vec_new(sizeof(int));
 * ```
````

- Headings (`#`, `##`, ...) only in file comments.
- No HTML tags (`<tt>`, `<b>`, ...): use `` `code` `` and `**bold**`.
- No `@code`/`@endcode`: use fences.

### DC13. Language

English. Full sentences ending with a period, except in member docs (DC2)
and `@param` descriptions, which may be phrases. (Not checked.)

---

## 9. Macro-generated APIs

### DC14. Macro-generated declarations

COCADA generates families of typed functions and types with macros:

```c
#define DECL_VEC_PUSH( TYPE ) \
	void vec_push_##TYPE(vec *v, TYPE val);

#define DECL_TYPED_VEC( TYPE , ...) \
	DECL_VEC_NEW(TYPE) \
	DECL_VEC_PUSH(TYPE) \
	...

XX_CORETYPES(DECL_TYPED_VEC)   // vec_push_int, vec_push_double, ...
```

- Macros that generate declarations are named `DECL_*`, and those that
  generate definitions (in `.c` files) `IMPL_*`, e.g. `DECL_HASHMAP_GET`
  and `IMPL_HASHMAP_GET`.
- A **leaf generator** declares types or functions (like
  `DECL_VEC_PUSH`). A **composite generator** only invokes other
  generators (like `DECL_TYPED_VEC`). A type list such as `XX_CORETYPES`
  invokes a generator for each type.

**The doc comment above a generator's `#define` documents the macro**, like
any other macro (DC4): `@param`s for the macro's own parameters. **The
generated declarations are documented inside the body**, each by a doc
comment before it, with the usual form and rules (`@param`s for the
parameters of the generated function, `@p`, `@return`, member docs, and so
on). Each line of a doc comment in a body ends with the `\` continuation, as
the other lines of the body. The generator's parameters (`TYPE`) can be
mentioned as placeholders.

```c
/**
 * @brief Declares an OK-Result type `NAME_res`.
 * @param NAME The type name prefix.
 * @param OK_TYPE The type of the result value.
 */
#define DECL_RESULT_OK(NAME, OK_TYPE) \
	/**\
	 * @brief OK-Result type.\
	 */\
	typedef struct {\
		bool ok;      /**< Success/fail indicator */\
		OK_TYPE val;  /**< Successful result value */\
	} NAME##_res;
```

- A composite generator has no declarations of its own to document; it is
  documented as a macro, or hidden with `@hide`.
- A doc comment above an invocation (e.g. `DECL_TRAIT(vec_iter, iter)`) is
  not used: the generated declarations are documented in the generator.
- `IMPL_*` generators define what `DECL_*` generators declare: only the
  macro is documented.

`cocadoc` expands the file-scope invocations of the documented headers, and
documents each **family** once, in the sections *Macro-generated types and
constants* and *Macro-generated functions* of the page of the header with
the invocation:

- the family is shown with the generator's parameters as placeholders
  (`void vec_push_TYPE(vec *v, TYPE val);`), followed by the invocation
  and the list of generated names (`vec_push_uchar`, `vec_push_ushort`,
  ...);
- a family with a single declaration (e.g. `semver_res`, from
  `DECL_RESULT_OK(semver, semver *)`) is shown as an ordinary item;
- references to generated names (`#vec_push_int`) link to their family;
- the generator macros are listed in the *Macros* section of the page of
  the header that defines them, with what they declare, unless hidden with
  `@hide`. A `@hide` in the body hides the generated declaration that
  follows it.

The expansion handles parameters, `__VA_ARGS__`, token pasting (`##`) and
nested macro calls. It does not evaluate `#if`, and uses the first
definition of a macro defined more than once.

---

## 10. Documentation pages

Each header gets one documentation page, which shows its authors and AI
involvement (DC15), with these sections:

1. The module documentation (the file comment).
2. Types, enums, unions and constants.
   1. Macro-generated types, enums, unions and constants.
3. Functions.
   1. Macro-generated functions.
4. Macros.

---

## 11. Summary

| rule | `cocadoc` warns about |
|---|---|
| DC1 | one-line comments; text on the `/**` or `*/` line; `**/` |
| DC2 | member docs on several lines, with block commands, or not as a trailing `/**<` |
| DC3 | no file comment; `@file` name different from the file name; no `@author` |
| DC15 | unknown `@ai` level; no AI named; `@ai` outside a file comment |
| DC4 | undocumented public declarations, members and macro-generated declarations |
| DC5 | missing or empty `@brief`; briefs with more than one sentence |
| DC6 | commands other than those of DC6, including typos like `@src` |
| DC7 | sections out of order |
| DC8 | `@param` syntax; parameters undocumented, unknown or out of order; missing `@return`, or `@return` on `void` |
| DC9 | old ownership annotations; misplaced `@move` |
| DC10 | anything but names in `@see` |
| DC11 | `::name` and `name()` references |
| DC12 | HTML tags; headings outside file comments; `@code`; unterminated code blocks |
| DC14 | docs of invocations (not used); invocations that cannot be expanded |

Warnings in the current code, over the 68 public headers (2026-10-05):

| rule | DC1 | DC3 | DC4 | DC5 | DC6 | DC7 | DC8 | DC9 | DC10 | DC11 | DC12 | DC14 | DC15 | total |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| warnings | 10 | 11 | 354 | 185 | 34 | 22 | 1595 | 42 | 82 | 114 | 99 | 0 | 0 | 2548 |

Most DC8 warnings are undocumented parameters (about 1200) and return values
(about 360) of functions that only have a `@brief`.
