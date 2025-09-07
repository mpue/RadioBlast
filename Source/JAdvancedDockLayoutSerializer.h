/*
  ==============================================================================

    JAdvancedDockLayoutSerializer.h
    Created: 29 Aug 2025 6:40:50pm
    Author:  mpue

  ==============================================================================
*/

/*
  ==============================================================================

    JAdvancedDockLayoutSerializer.h
    Created: 29 Aug 2025

    Simple utility class for saving/loading JAdvancedDock column and row sizes
    For static dock layouts where components never change

  ==============================================================================
*/

#pragma once

#include "JAdvancedDock.h"
#include <JuceHeader.h>

/**
 * Simple serializer for JAdvancedDock layouts that only saves/loads sizes
 * Perfect for static layouts where components are always the same
 */
class JAdvancedDockLayoutSerializer
{
public:
    /**
     * Structure to hold dock sizing information
     */
    struct DockSizes
    {
        struct ColumnSize
        {
            double width;

            ColumnSize(double w = 200.0) : width(w) {}
        };

        struct RowSize
        {
            std::vector<ColumnSize> columns;
            double height;

            RowSize(double h = 200.0) : height(h) {}

            void addColumn(double width)
            {
                columns.emplace_back(width);
            }
        };

        std::vector<RowSize> rows;

        void addRow(double height = 200.0)
        {
            rows.emplace_back(height);
        }

        RowSize& getRow(int index)
        {
            return rows[index];
        }

        bool isEmpty() const
        {
            return rows.empty();
        }

        void clear()
        {
            rows.clear();
        }
    };

public:
    /**
     * Extract current sizes from dock
     * @param dock The dock to read sizes from
     * @return DockSizes structure with current layout
     */
    static DockSizes extractSizes(const JAdvancedDock& dock)
    {
        DockSizes sizes;

        int rowCount = dock.getRowCount();

        for (int row = 0; row < rowCount; ++row)
        {
            sizes.addRow(200.0); // Default height - könnte erweitert werden wenn getRowHeight() verfügbar ist

            int columnCount = dock.getColumnCountInRow(row);

            for (int col = 0; col < columnCount; ++col)
            {
                double width = dock.getColumnWidth(row, col);
                sizes.getRow(row).addColumn(width);
            }
        }

        return sizes;
    }

    /**
     * Apply sizes to dock
     * @param dock The dock to modify
     * @param sizes The sizes to apply
     * @return True if successful
     */
    static bool applySizes(JAdvancedDock& dock, const DockSizes& sizes)
    {
        if (sizes.isEmpty())
            return true;

        // Check if dock structure matches saved structure
        if (dock.getRowCount() != sizes.rows.size())
        {
            DBG("Row count mismatch: dock has " << dock.getRowCount()
                << " rows, but sizes have " << sizes.rows.size() << " rows");
            return false;
        }

        // Apply column widths for each row
        for (int row = 0; row < sizes.rows.size(); ++row)
        {
            const auto& rowSize = sizes.rows[row];
            int dockColumnCount = dock.getColumnCountInRow(row);

            if (dockColumnCount != rowSize.columns.size())
            {
                DBG("Column count mismatch in row " << row
                    << ": dock has " << dockColumnCount
                    << " columns, but sizes have " << rowSize.columns.size() << " columns");
                return false;
            }

            // Set each column width
            for (int col = 0; col < rowSize.columns.size(); ++col)
            {
                dock.setColumnWidth(row, col, rowSize.columns[col].width);
            }
        }

        return true;
    }

    /**
     * Save sizes to XML
     * @param sizes The sizes to save
     * @return XmlElement with the serialized sizes
     */
    static std::unique_ptr<XmlElement> sizesToXml(const DockSizes& sizes)
    {
        auto xml = std::make_unique<XmlElement>("DockLayout");
        xml->setAttribute("version", "1.0");
        xml->setAttribute("timestamp", String(Time::getCurrentTime().toMilliseconds()));

        for (int rowIndex = 0; rowIndex < sizes.rows.size(); ++rowIndex)
        {
            const auto& row = sizes.rows[rowIndex];
            auto* rowXml = xml->createNewChildElement("Row");
            rowXml->setAttribute("index", rowIndex);
            rowXml->setAttribute("height", row.height);

            for (int colIndex = 0; colIndex < row.columns.size(); ++colIndex)
            {
                const auto& column = row.columns[colIndex];
                auto* colXml = rowXml->createNewChildElement("Column");
                colXml->setAttribute("index", colIndex);
                colXml->setAttribute("width", column.width);
            }
        }

        return xml;
    }

    /**
     * Load sizes from XML
     * @param xml The XML element to load from
     * @return DockSizes structure
     */
    static DockSizes sizesFromXml(const XmlElement& xml)
    {
        DockSizes sizes;

        if (xml.getTagName() != "DockLayout")
            return sizes;

        for (auto* rowXml : xml.getChildWithTagNameIterator("Row"))
        {
            double height = rowXml->getDoubleAttribute("height", 200.0);
            sizes.addRow(height);

            int rowIndex = sizes.rows.size() - 1;

            for (auto* colXml : rowXml->getChildWithTagNameIterator("Column"))
            {
                double width = colXml->getDoubleAttribute("width", 200.0);
                sizes.getRow(rowIndex).addColumn(width);
            }
        }

        return sizes;
    }

    /**
     * Save sizes to file
     * @param sizes The sizes to save
     * @param file The file to save to
     * @return True if successful
     */
    static bool saveToFile(const DockSizes& sizes, const File& file)
    {
        auto xml = sizesToXml(sizes);
        if (xml)
        {
            return xml->writeTo(file, XmlElement::TextFormat().singleLine());
        }
        return false;
    }

    /**
     * Load sizes from file
     * @param file The file to load from
     * @return DockSizes structure (empty if loading failed)
     */
    static DockSizes loadFromFile(const File& file)
    {
        if (!file.exists())
            return DockSizes();

        auto xml = XmlDocument::parse(file);
        if (xml)
        {
            return sizesFromXml(*xml);
        }

        return DockSizes();
    }

    /**
     * Save current dock layout to file
     * @param dock The dock to save
     * @param file The file to save to
     * @return True if successful
     */
    static bool saveDockLayout(const JAdvancedDock& dock, const File& file)
    {
        auto sizes = extractSizes(dock);
        return saveToFile(sizes, file);
    }

    /**
     * Load and apply dock layout from file
     * @param dock The dock to modify
     * @param file The file to load from
     * @return True if successful
     */
    static bool loadDockLayout(JAdvancedDock& dock, const File& file)
    {
        auto sizes = loadFromFile(file);
        return applySizes(dock, sizes);
    }

    /**
     * Create a string representation of the current layout for debugging
     * @param dock The dock to analyze
     * @return String describing the layout
     */
    static String getLayoutDescription(const JAdvancedDock& dock)
    {
        String description;
        description << "Dock Layout:\n";
        description << "Rows: " << dock.getRowCount() << "\n";

        for (int row = 0; row < dock.getRowCount(); ++row)
        {
            description << "  Row " << row << " (columns: " << dock.getColumnCountInRow(row) << "):\n";

            for (int col = 0; col < dock.getColumnCountInRow(row); ++col)
            {
                description << "    Column " << col << ": width=" << dock.getColumnWidth(row, col) << "\n";
            }
        }

        return description;
    }

    /**
     * Validate that dock structure matches saved sizes
     * @param dock The dock to check
     * @param sizes The sizes to compare against
     * @return True if structures match
     */
    static bool validateStructure(const JAdvancedDock& dock, const DockSizes& sizes)
    {
        if (dock.getRowCount() != sizes.rows.size())
            return false;

        for (int row = 0; row < sizes.rows.size(); ++row)
        {
            if (dock.getColumnCountInRow(row) != sizes.rows[row].columns.size())
                return false;
        }

        return true;
    }
};