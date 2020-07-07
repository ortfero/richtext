#pragma once


#include <string>
#include <list>
#include <variant>
#include <memory>
#include <system_error>


namespace richtext {


enum class tag {
  undefined, normal, emphasis, strong
};


class span {
public:

  span() noexcept = default;
  span(span const&) = default;
  span& operator = (span const&) = default;
  span(span&&) noexcept = default;
  span& operator = (span&&) noexcept = default;
  enum tag tag() const noexcept { return tag_; }
  std::string const& text() const noexcept { return text_; }

  span(enum tag tag, std::string text) noexcept:
    tag_{tag}, text_{std::move(text)}
  { }

private:

  enum tag tag_;
  std::string text_;
};


class text {
public:

  using items_type = std::list<span>;
  using const_iterator = items_type::const_iterator;

  text() = default;
  text(text const&) = default;
  text& operator = (text const&) = default;
  text(text&&) = default;
  text& operator = (text&&) = default;
  const_iterator begin() const noexcept { return items_.begin(); }
  const_iterator end() const noexcept { return items_.end(); }
  bool empty() const noexcept { return items_.empty(); }
  void add(span span) { items_.emplace_back(std::move(span)); }

private:

  items_type items_;
};


class paragraph {
public:

  paragraph() = default;
  paragraph(paragraph const&) = default;
  paragraph& operator = (paragraph const&) = default;
  paragraph(paragraph&&) = default;
  paragraph& operator = (paragraph&&) = default;
  explicit paragraph(text text) noexcept: text_{std::move(text)} { }
  text const& text() const noexcept { return text_; }

private:

  class text text_;
};


class table {
public:

  using row= std::vector<text>;
  using rows = std::list<row>;
  using const_iterator = rows::const_iterator;

  table() = default;
  table(table const&) = default;
  table& operator = (table const&) = default;
  table(table&&) = default;
  table& operator = (table&&) = default;
  explicit table(row header) noexcept: header_{std::move(header)} { }
  row const& header() const noexcept { return header_; }
  const_iterator begin() const noexcept { return rows_.begin(); }
  const_iterator end() const noexcept { return rows_.end(); }
  void add(row row) { rows_.emplace_back(std::move(row)); }

private:

  row header_;
  rows rows_;
};


class fragment;
using fragment_ptr = std::unique_ptr<fragment>;

class ordered_list;


class unordered_list {
public:

  using items_type = std::list<fragment_ptr>;
  using const_iterator = items_type::const_iterator;

  unordered_list() = default;
  unordered_list(unordered_list const&) = default;
  unordered_list& operator = (unordered_list const&) = default;
  unordered_list(unordered_list&&) noexcept = default;
  unordered_list& operator = (unordered_list&&) noexcept = default;
  const_iterator begin() const noexcept { return items_.begin(); }
  const_iterator end() const noexcept { return items_.end(); }
  void add(paragraph);
  void add(unordered_list);
  void add(ordered_list);

private:

  items_type items_;
};


class ordered_list {
public:

  using items_type = std::list<fragment_ptr>;
  using const_iterator = items_type::const_iterator;

  ordered_list() = default;
  ordered_list(ordered_list const&) = default;
  ordered_list& operator = (ordered_list const&) = default;
  ordered_list(ordered_list&&) noexcept = default;
  ordered_list& operator = (ordered_list&&) noexcept = default;
  const_iterator begin() const noexcept { return items_.begin(); }
  const_iterator end() const noexcept { return items_.end(); }
  void add(paragraph);
  void add(unordered_list);
  void add(ordered_list);

private:

  items_type items_;
};


enum class fragment_kind {
  undefined, paragraph, table, unordered_list, ordered_list, subsection, section
};


class fragment {
public:

  using item_type = std::variant<std::monostate, class paragraph, class table,
                                 unordered_list, ordered_list>;

  fragment() = default;
  fragment(fragment const&) = default;
  fragment& operator = (fragment const&) = default;
  fragment(fragment&&) = default;
  fragment& operator = (fragment&&) = default;
  explicit fragment(paragraph paragraph) noexcept: item_{std::move(paragraph)} { }
  explicit fragment(table table) noexcept: item_{std::move(table)} { }
  explicit fragment(unordered_list unordered_list) noexcept: item_{std::move(unordered_list)} { }
  explicit fragment(ordered_list ordered_list) noexcept: item_{std::move(ordered_list)} { }
  paragraph const* paragraph() const noexcept { return std::get_if<class paragraph>(&item_); }
  table const* table() const noexcept { return std::get_if<class table>(&item_); }
  unordered_list const* unordered_list() const noexcept { return std::get_if<class unordered_list>(&item_); }
  ordered_list const* ordered_list() const noexcept { return std::get_if<class ordered_list>(&item_); }

  fragment_kind kind() const noexcept {
    switch(item_.index()) {
      case 1  : return fragment_kind::paragraph;
      case 2  : return fragment_kind::table;
      case 3  : return fragment_kind::unordered_list;
      case 4  : return fragment_kind::ordered_list;
      default : return fragment_kind::undefined;
    }
  }

private:

  item_type item_;
};


inline void unordered_list::add(paragraph paragraph) {
  items_.emplace_back(std::make_unique<fragment>(std::move(paragraph)));
}


inline void unordered_list::add(unordered_list unordered_list) {
  items_.emplace_back(std::make_unique<fragment>(std::move(unordered_list)));
}


inline void unordered_list::add(ordered_list ordered_list) {
  items_.emplace_back(std::make_unique<fragment>(std::move(ordered_list)));
}


inline void ordered_list::add(paragraph paragraph) {
  items_.emplace_back(std::make_unique<fragment>(std::move(paragraph)));
}


inline void ordered_list::add(unordered_list unordered_list) {
  items_.emplace_back(std::make_unique<fragment>(std::move(unordered_list)));
}


inline void ordered_list::add(ordered_list ordered_list) {
  items_.emplace_back(std::make_unique<fragment>(std::move(ordered_list)));
}


class subsection {
public:
  using items_type = std::list<fragment>;
  using const_iterator = items_type::const_iterator;

  subsection() = default;
  subsection(subsection const&) = default;
  subsection& operator = (subsection const&) = default;
  subsection(subsection&&) = default;
  subsection& operator = (subsection&&) = default;
  explicit subsection(text header) noexcept: header_{std::move(header)} { }
  const_iterator begin() const noexcept { return items_.begin(); }
  const_iterator end() const noexcept { return items_.end(); }
  text const& header() const noexcept { return header_; }
  void add(paragraph paragraph) { items_.emplace_back(fragment{std::move(paragraph)}); }
  void add(table table) { items_.emplace_back(fragment{std::move(table)}); }
  void add(unordered_list unordered_list) { items_.emplace_back(fragment{std::move(unordered_list)}); }
  void add(ordered_list ordered_list) { items_.emplace_back(fragment{std::move(ordered_list)}); }

private:

  text header_;
  items_type items_;
};


class subsection_or_fragment {
public:

  using item_type = std::variant<std::monostate, class paragraph, class table,
                                 class unordered_list, class ordered_list,
                                 class subsection>;

  subsection_or_fragment() = default;
  subsection_or_fragment(subsection_or_fragment const&) = default;
  subsection_or_fragment& operator = (subsection_or_fragment const&) = default;
  subsection_or_fragment(subsection_or_fragment&&) = default;
  subsection_or_fragment& operator = (subsection_or_fragment&&) = default;
  explicit subsection_or_fragment(paragraph paragraph) noexcept: item_{std::move(paragraph)} { }
  explicit subsection_or_fragment(table table) noexcept: item_{std::move(table)} { }
  explicit subsection_or_fragment(unordered_list unordered_list) noexcept: item_{std::move(unordered_list)} { }
  explicit subsection_or_fragment(ordered_list ordered_list) noexcept: item_{std::move(ordered_list)} { }
  explicit subsection_or_fragment(subsection subsection) noexcept: item_{std::move(subsection)} { }
  paragraph const* paragraph() const noexcept { return std::get_if<class paragraph>(&item_); }
  table const* table() const noexcept { return std::get_if<class table>(&item_); }
  unordered_list const* unordered_list() const noexcept { return std::get_if<class unordered_list>(&item_); }
  ordered_list const* ordered_list() const noexcept { return std::get_if<class ordered_list>(&item_); }
  subsection const* subsection() const noexcept { return std::get_if<class subsection>(&item_); }

  fragment_kind kind() const noexcept {
    switch(item_.index()) {
      case 1  : return fragment_kind::paragraph;
      case 2  : return fragment_kind::table;
      case 3  : return fragment_kind::unordered_list;
      case 4  : return fragment_kind::ordered_list;
      case 5  : return fragment_kind::subsection;
      default : return fragment_kind::undefined;
    }
  }

private:

  item_type item_;
};


class section {
public:
  using items_type = std::list<subsection_or_fragment>;
  using const_iterator = items_type::const_iterator;

  section() = default;
  section(section const&) = default;
  section& operator = (section const&) = default;
  section(section&&) = default;
  section& operator = (section&&) = default;
  explicit section(text header) noexcept: header_{std::move(header)} { }
  const_iterator begin() const noexcept { return items_.begin(); }
  const_iterator end() const noexcept { return items_.end(); }
  text const& header() const noexcept { return header_; }
  void add(paragraph paragraph) { items_.emplace_back(subsection_or_fragment{std::move(paragraph)}); }
  void add(table table) { items_.emplace_back(subsection_or_fragment{std::move(table)}); }
  void add(unordered_list unordered_list) { items_.emplace_back(subsection_or_fragment{std::move(unordered_list)}); }
  void add(ordered_list ordered_list) { items_.emplace_back(subsection_or_fragment{std::move(ordered_list)}); }
  void add(subsection subsection) { items_.emplace_back(subsection_or_fragment{std::move(subsection)}); }


private:

  text header_;
  items_type items_;
};


class section_or_fragment {
public:

  using item_type = std::variant<std::monostate, class paragraph,
                                 class table, class unordered_list, class ordered_list,
                                 class subsection, class section>;

  section_or_fragment() = default;
  section_or_fragment(section_or_fragment const&) = default;
  section_or_fragment& operator = (section_or_fragment const&) = default;
  section_or_fragment(section_or_fragment&&) = default;
  section_or_fragment& operator = (section_or_fragment&&) = default;
  explicit section_or_fragment(paragraph paragraph) noexcept: item_{std::move(paragraph)} { }
  explicit section_or_fragment(table table) noexcept: item_{std::move(table)} { }
  explicit section_or_fragment(unordered_list unordered_list) noexcept: item_{std::move(unordered_list)} { }
  explicit section_or_fragment(ordered_list ordered_list) noexcept: item_{std::move(ordered_list)} { }
  explicit section_or_fragment(subsection subsection) noexcept: item_{std::move(subsection)} { }
  explicit section_or_fragment(section section) noexcept: item_{std::move(section)} { }
  paragraph const* paragraph() const noexcept { return std::get_if<class paragraph>(&item_); }
  table const* table() const noexcept { return std::get_if<class table>(&item_); }
  unordered_list const* unordered_list() const noexcept { return std::get_if<class unordered_list>(&item_); }
  ordered_list const* ordered_list() const noexcept { return std::get_if<class ordered_list>(&item_); }
  subsection const* subsection() const noexcept { return std::get_if<class subsection>(&item_); }
  section const* section() const noexcept { return std::get_if<class section>(&item_); }

  fragment_kind kind() const noexcept {
    switch(item_.index()) {
      case 1  : return fragment_kind::paragraph;
      case 2  : return fragment_kind::table;
      case 3  : return fragment_kind::unordered_list;
      case 4  : return fragment_kind::ordered_list;
      case 5  : return fragment_kind::subsection;
      case 6  : return fragment_kind::section;
      default : return fragment_kind::undefined;
    }
  }

private:

  item_type item_;
};


class document {
public:
  using items_type = std::list<section_or_fragment>;
  using const_iterator = items_type::const_iterator;

  document() = default;
  document(document const&) = default;
  document& operator = (document const&) = default;
  document(document&&) = default;
  document& operator = (document&&) = default;
  explicit document(text header) noexcept: header_{std::move(header)} { }
  const_iterator begin() const noexcept { return items_.begin(); }
  const_iterator end() const noexcept { return items_.end(); }
  text const& header() const noexcept { return header_; }
  void add(paragraph paragraph) { items_.emplace_back(section_or_fragment{std::move(paragraph)}); }
  void add(table table) { items_.emplace_back(section_or_fragment{std::move(table)}); }
  void add(unordered_list unordered_list) { items_.emplace_back(section_or_fragment{std::move(unordered_list)}); }
  void add(ordered_list ordered_list) { items_.emplace_back(section_or_fragment{std::move(ordered_list)}); }
  void add(subsection subsection) { items_.emplace_back(section_or_fragment{std::move(subsection)}); }
  void add(section section) { items_.emplace_back(section_or_fragment{std::move(section)}); }

private:

  text header_;
  items_type items_;

};


class formatter {
public:


  bool render(document const& document, std::error_code& ec) {

    if(!on_document_begin(document, ec))
      return false;

    if(!document.header().empty()) {
      on_document_header_begin(document.header());
      on_text(document.header());
      on_document_header_end(document.header());
    }        

    for(auto const& section_or_fragment: document)
      switch(section_or_fragment.kind()) {
        case fragment_kind::paragraph:
          render(*section_or_fragment.paragraph());
          continue;
        case fragment_kind::table:
          render(*section_or_fragment.table());
          continue;
        case fragment_kind::unordered_list:
          render(*section_or_fragment.unordered_list());
          continue;
        case fragment_kind::ordered_list:
          render(*section_or_fragment.ordered_list());
          continue;
        case fragment_kind::subsection:
          render(*section_or_fragment.subsection());
          continue;
        case fragment_kind::section:
          render(*section_or_fragment.section());
          continue;
        default:
          return false;
      }

    if(!on_document_end(document, ec))
      return false;
  }


protected:

  virtual bool on_document_begin(document const&, std::error_code&) { return true; }  
  virtual bool on_document_end(document const&, std::error_code&) { return true; }
  virtual void on_document_header_begin(text const&) { }
  virtual void on_document_header_end(text const&) { }
  virtual void on_text(text const&) { }
  virtual void on_paragraph_begin(paragraph const&) { }
  virtual void on_paragraph_end(paragraph const&) { }
  virtual void on_table_begin(table const&) { }
  virtual void on_table_end(table const&) { }
  virtual void on_table_header_begin(table::row const&) { }
  virtual void on_table_header_end(table::row const&) { }
  virtual void on_table_header_cell_begin(std::size_t, text const&) { }
  virtual void on_table_header_cell_end(std::size_t, text const&) { }
  virtual void on_table_header_cell_text(std::size_t, text const&) { }
  virtual void on_table_row_begin(table::row const&) { }
  virtual void on_table_row_end(table::row const&) { }
  virtual void on_table_cell_begin(std::size_t, text const&) { }
  virtual void on_table_cell_end(std::size_t, text const&) { }
  virtual void on_table_cell_text(std::size_t, text const&) { }
  virtual void on_subsection_begin(subsection const&) { }
  virtual void on_subsection_end(subsection const&) { }
  virtual void on_subsection_header_begin(text const&) { }
  virtual void on_subsection_header_end(text const&) { }
  virtual void on_section_begin(section const&) { }
  virtual void on_section_end(section const&) { }
  virtual void on_section_header_begin(text const&) { }
  virtual void on_section_header_end(text const&) { }
  virtual void on_unordered_list_begin(unordered_list const&) { }
  virtual void on_unordered_list_end(unordered_list const&) { }
  virtual void on_unordered_list_item_begin(fragment const&) { }
  virtual void on_unordered_list_item_end(fragment const&) { }
  virtual void on_ordered_list_begin(ordered_list const&) { }
  virtual void on_ordered_list_end(ordered_list const&) { }
  virtual void on_ordered_list_item_begin(ordered_list const&) { }
  virtual void on_ordered_list_item_end(ordered_list const&) { }

private:


  void render(paragraph const& paragraph) {
    on_paragraph_begin(paragraph);
    on_text(paragraph.text());
    on_paragraph_end(paragraph);
  }


  void render(table const& table) {
    on_table_begin(table);

    if(!table.header().empty()) {
      on_table_header_begin(table.header());
      std::size_t i = 0;
      for(auto const& column: table.header()) {
        on_table_header_cell_begin(i, column);
        on_table_header_cell_text(i, column);
        on_table_header_cell_end(i, column);
      }
      on_table_header_end(table.header());
    }

    for(auto const& row: table) {
      on_table_row_begin(row);

      std::size_t i = 0;
      for(auto const& cell: row) {
        on_table_cell_begin(i, cell);
        on_table_cell_text(i, cell);
        on_table_cell_end(i, cell);
        ++i;
      }

      on_table_row_end(row);
    }

    on_table_end(table);
  }


  void render(unordered_list const& unordered_list) {
    on_unordered_list_begin(unordered_list);

    for(auto const& item: unordered_list) {
      on_unordered_list_item_begin(*item);

      switch(item->kind()) {
        case fragment_kind::paragraph:
          on_text(item->paragraph()->text());
          continue;
        case fragment_kind::unordered_list:
          render(*item->unordered_list());
          continue;
        case fragment_kind::ordered_list:
          render(*item->ordered_list());
          continue;
        default:
          continue;
      }
      
      on_unordered_list_item_end(*item);
    }

    on_unordered_list_end(unordered_list);
  }


  void render(ordered_list const& ordered_list) {    
    on_ordered_list_begin(ordered_list);

    for(auto const& item: ordered_list) {
      on_ordered_list_item_begin(ordered_list);

      switch(item->kind()) {
        case fragment_kind::paragraph:
          on_text(item->paragraph()->text());
          continue;
        case fragment_kind::unordered_list:
          render(*item->unordered_list());
          continue;
        case fragment_kind::ordered_list:
          render(*item->ordered_list());
          continue;
        default:
          continue;
      }
      
      on_ordered_list_item_end(ordered_list);
    }

    on_ordered_list_end(ordered_list);
  }


  void render(subsection const& subsection) {
    on_subsection_begin(subsection);

    if(!subsection.header().empty()) {
      on_subsection_header_begin(subsection.header());
      on_text(subsection.header());
      on_subsection_header_end(subsection.header());
    }

    for(auto const& fragment: subsection)
      switch(fragment.kind()) {
        case fragment_kind::paragraph:
          render(*fragment.paragraph());
          continue;
        case fragment_kind::table:
          render(*fragment.table());
          continue;
        case fragment_kind::unordered_list:
          render(*fragment.unordered_list());
          continue;
        case fragment_kind::ordered_list:
          render(*fragment.ordered_list());
          continue;
        default:
          continue;
      }

    on_subsection_end(subsection);
  }


  void render(section const& section) {
    on_section_begin(section);

    if(!section.header().empty()) {
      on_section_header_begin(section.header());
      on_text(section.header());
      on_section_header_end(section.header());
    }
    
    for(auto const& subsection_or_fragment: section)
      switch(subsection_or_fragment.kind()) {
        case fragment_kind::paragraph:
          render(*subsection_or_fragment.paragraph());
          continue;
        case fragment_kind::table:
          render(*subsection_or_fragment.table());
          continue;
        case fragment_kind::unordered_list:
          render(*subsection_or_fragment.unordered_list());
          continue;
        case fragment_kind::ordered_list:
          render(*subsection_or_fragment.ordered_list());
          continue;
        case fragment_kind::subsection:
          render(*subsection_or_fragment.subsection());
          continue;
        default:
          continue;
      }

    on_section_end(section);
  }

};



}
