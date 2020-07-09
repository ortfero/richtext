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
  explicit operator bool() const noexcept { return file_ != nullptr; }


  markdown(std::string const& filename, options const& opts = options{}) noexcept:
    options_{ opts } {
    file_ = fopen(filename.data(), "wb+");
    if (file_ == nullptr)
      return;
  }

  void on_document_header(std::string const& header) noexcept override {
    fprintf(file_, "\n# %s\n\n", header.data());
  }

  void on_section_header(std::string const& header) noexcept override {
    fprintf(file_, "\n## %s\n\n", header.data());
  }

  void on_subsection_header(std::string const& header) noexcept override {
    fprintf(file_, "\n### %s\n\n", header.data());
  }


  void on_text(text const& text) override {
    for (auto const& span : text)
      on_span(span);
  }


  void on_paragraph_end(paragraph const&) override {
    fputc('\n', file_);
    fputc('\n', file_);
  }


  void on_table_begin(table const& table) override {
    table_stack_.emplace(&table);
  }


  void on_table_end(table const&) override {
    table_stack_.pop();
    fputc('\n', file_);
  }


  void on_table_header_begin(table_header const&) override {
    indent();
    fputc('|', file_);
  }


  void on_table_header_end(table_header const& header) override {
    fputc('\n', file_);
    indent();
    fputc('|', file_);
    for (std::size_t i = 0; i != header.size(); ++i)
      fprintf(file_, "%*c|", header[i].size() + 2, '-');
    fputc('\n', file_);
  }


  void on_table_header_cell(std::size_t, std::string const& text) override {
    fprintf(file_, " %s |", text.data());
  }


  void on_table_row_begin(table_row const&) override {
    indent();
    fputc('|', file_);
    fputc(' ', file_);
  }


  void on_table_row_end(table_row const&) override {
    fputc('\n', file_);
  }


  void on_table_cell_end(std::size_t, text const&) override {
    fputc(' ', file_);
    fputc('|', file_);
  }


  void on_table_cell_text(std::size_t, text const& text) override {
    on_text(text);
  }


  void on_unordered_list_header(std::string const& header) override {
    indent();
    fputs(header.data(), file_);
    fputc('\n', file_);
  }


  void on_unordered_list_item_begin(fragment const&) override {
    indent();
    fputc('-', file_);
    fputc(' ', file_);
    indent_ += options_.indent();
  }


  void on_unordered_list_item_end(fragment const&) override {
    indent_ -= options_.indent();
    fputc('\n', file_);
  }


  void on_ordered_list_header(std::string const& header) override {
    indent();
    fputs(header.data(), file_);
    fputc('\n', file_);
  }

  void on_ordered_list_item_begin(std::size_t i, fragment const&) override {
    indent();
    fprintf(file_, "%d. ", i);
    indent_ += options_.indent();
  }


  void on_ordered_list_item_end(std::size_t, fragment const&) override {
    indent_ -= options_.indent();
    fputc('\n', file_);
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
    fprintf(file_, "%*c", indent_, ' ');
  }


  void on_span(span const& span) {
    switch (span.tag()) {
    case tag::strong:
      fprintf(file_, "**%s**", span.text().data());
      return;
    case tag::emphasis:
      fprintf(file_, "*%s*", span.text().data());
      return;
    case tag::strong_emphasis:
      fprintf(file_, "***%s***", span.text().data());
      return;
    default:
      fputs(span.text().data(), file_);
      return;
    }
  }

};



}
