#pragma once


#include <string>
#include <cstdio>
#include <vector>
#include <stack>
#include "../richtext.hpp"


namespace richtext::formatters {


class markdown: public formatter {
public:

  class options {
  public:

    std::size_t static constexpr default_margin = 80;
    std::size_t static constexpr default_indent = 4;

    options() noexcept = default;
    options(options const&) noexcept = default;
    options& operator = (options const&) noexcept = default;

    options& margin(std::size_t margin) noexcept { margin_ = margin; }
    options& indent(std::size_t indent) noexcept { indent_ = indent; }

    std::size_t magin() const noexcept { return margin_; }
    std::size_t indent() const noexcept { return indent_; }

  private:

    std::size_t margin_{ default_margin };
    std::size_t indent_{ default_indent };
  };

  markdown() noexcept = default;
  markdown(markdown const&) = delete;
  markdown& operator = (markdown const&) = delete;


  explicit markdown(options const& options) noexcept: options_{ options } { }

  void on_document_header(std::string const& header) noexcept override {
    texter() << '\n' << '#' << ' ' << header << '\n' << '\n';
  }

  void on_section_header(std::string const& header) noexcept override {
    texter() << '\n' << '#' << '#' << ' ' << header << '\n' << '\n';
  }

  void on_subsection_header(std::string const& header) noexcept override {
    texter() << '\n' << '#' << '#' << '#' << ' ' << header << '\n' << '\n';
  }


  void on_text(text const& text) override {
    for (auto const& span : text)
      on_span(span);
  }


  void on_paragraph_end(paragraph const&) override {
    texter() << '\n' << '\n';
  }


  void on_table_begin(table const& table) override {
    table_stack_.emplace(&table);
  }


  void on_table_end(table const&) override {
    table_stack_.pop();
    texter() << '\n';
  }


  void on_table_header_begin(table_header const&) override {
    indent();
    texter() << '|';
  }


  void on_table_header_end(table_header const& header) override {
    texter() << '\n';
    indent();
    texter() << '|';
    for (std::size_t i = 0; i != header.size(); ++i)
      texter().char_n('-', header[i].size() + 2).print('|');
    texter() << '\n';
  }


  void on_table_header_cell(std::size_t, std::string const& text) override {
    texter() << ' ' << text << ' ' << '|';
  }


  void on_table_row_begin(table_row const&) override {
    indent();
    texter() << '|' << ' ';
  }


  void on_table_row_end(table_row const&) override {
    texter() << '\n';
  }


  void on_table_cell_end(std::size_t, text const&) override {
    texter() << ' ' << '|';
  }


  void on_table_cell_text(std::size_t, text const& text) override {
    on_text(text);
  }


  void on_unordered_list_end(unordered_list const&) override {
    texter() << '\n';
  }


  void on_unordered_list_header(std::string const& header) override {
    indent();
    texter() << header << '\n';
  }


  void on_unordered_list_item_begin(fragment const&) override {
    indent();
    texter() << '-' << ' ';
    indent_ += options_.indent();
  }


  void on_unordered_list_item_end(fragment const&) override {
    indent_ -= options_.indent();
    texter() << '\n';
  }


  void on_ordered_list_end(ordered_list const&) override {
    texter() << '\n';
  }


  void on_ordered_list_header(std::string const& header) override {
    indent();
    texter() << header << '\n';
  }

  void on_ordered_list_item_begin(std::size_t i, fragment const&) override {
    indent();
    texter() << i << '.' << ' ';
    indent_ += options_.indent();
  }


  void on_ordered_list_item_end(std::size_t, fragment const&) override {
    indent_ -= options_.indent();
    texter() << '\n';
  }
  

private:
  using table_stack = std::stack<table const*>;

  FILE* file_{nullptr};
  std::size_t indent_{ 0 };
  options options_;
  table_stack table_stack_;


  void indent() {
    if (indent_ == 0)
      return;
    texter().char_n(' ', indent_);
  }


  void on_span(span const& span) {
    switch (span.tag()) {
    case tag::strong:
      texter() << '*' << '*' << span.text() << '*' << '*';
      return;
    case tag::emphasis:
      texter() << '*' << span.text() << '*';
      return;
    case tag::strong_emphasis:
      texter() << '*' << '*' << '*' << span.text() << '*' << '*' << '*';
      return;
    default:
      texter() << span.text();
      return;
    }
  }

};



}
