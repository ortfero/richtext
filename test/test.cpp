#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <richtext/richtext.hpp>
#include <richtext/formatters/markdown.hpp>


TEST_CASE("richtext") {

  using namespace richtext;
  formatters::markdown md;
  auto doc = document{ "Document Header" }
    .add(paragraph{ "This is paragraph" })
    .add(paragraph{}
      .add("This ")
      .add(tag::strong, "is")
      .add(" ")
      .add(tag::emphasis, "formatted")
      .add(" ")
      .add(tag::strong_emphasis, "text"))
    .add(section{ "Section Header" }
      .add(unordered_list{ "Unordered items:" }
        .add(paragraph{ "Item 1" })
        .add(paragraph{ "Item 2" })
        .add(paragraph{ "Item 3" })))
    .add(subsection{ "Subsection Header" }
      .add(ordered_list{ "Ordered items:" }
        .add(paragraph{ "Item 1" })
        .add(paragraph{ "Item 2" }))
      .add(table{ {"Column A", "Column B"} }
        .add(table_row{}.add("1").add("2"))
        .add(table_row{}.add("3").add("4"))));
  md.render(doc);
  std::error_code ec; md.write("test.md", ec);
}
