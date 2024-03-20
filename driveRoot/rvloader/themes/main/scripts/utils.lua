function table.contains(table, element)
    for index, value in pairs(table) do
        if value == element then
            return index
        end
    end
    return nil
end