#include "highlighter_group.hh"

#include "containers.hh"

namespace Kakoune
{

void HighlighterGroup::highlight(const Context& context, HighlightPass pass,
                                 DisplayBuffer& display_buffer, BufferRange range)
{
    for (auto& hl : m_highlighters)
       hl.value->highlight(context, pass, display_buffer, range);
}

void HighlighterGroup::compute_display_setup(const Context& context, HighlightPass pass,
                                             DisplayCoord scroll_offset, DisplaySetup& setup)
{
    for (auto& hl : m_highlighters)
       hl.value->compute_display_setup(context, pass, scroll_offset, setup);
}

void HighlighterGroup::add_child(HighlighterAndId&& hl)
{
    hl.first = replace(hl.first, "/", "<slash>");

    if (m_highlighters.contains(hl.first))
        throw runtime_error(format("duplicate id: '{}'", hl.first));

    m_highlighters.insert({std::move(hl.first), std::move(hl.second)});
}

void HighlighterGroup::remove_child(StringView id)
{
    m_highlighters.remove(id);
}

Highlighter& HighlighterGroup::get_child(StringView path)
{
    auto sep_it = find(path, '/');
    StringView id(path.begin(), sep_it);
    auto it = m_highlighters.find(id);
    if (it == m_highlighters.end())
        throw child_not_found(format("no such id: '{}'", id));
    if (sep_it == path.end())
        return *it->value;
    else
        return it->value->get_child({sep_it+1, path.end()});
}

Completions HighlighterGroup::complete_child(StringView path, ByteCount cursor_pos, bool group) const
{
    auto sep_it = find(path, '/');
    if (sep_it != path.end())
    {
        ByteCount offset = sep_it+1 - path.begin();
        Highlighter& hl = const_cast<HighlighterGroup*>(this)->get_child({path.begin(), sep_it});
        return offset_pos(hl.complete_child(path.substr(offset), cursor_pos - offset, group), offset);
    }

    auto candidates = complete(
        path, cursor_pos,
        m_highlighters | filter([=](const HighlighterMap::Item& hl)
                                { return not group or hl.value->has_children(); })
                       | transform(std::mem_fn(&HighlighterMap::Item::key)));

    return { 0, 0, std::move(candidates) };
}

}
