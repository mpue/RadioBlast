/*
  ==============================================================================

    DockLayoutManager.h
    Created: 29 Aug 2025 6:41:06pm
    Author:  mpue

  ==============================================================================
*/

/*
  ==============================================================================

    DockLayoutManager.h
    Created: 29 Aug 2025

    Simple manager class for handling dock layout persistence
    Usage example for JAdvancedDockLayoutSerializer

  ==============================================================================
*/

#pragma once

#include "JAdvancedDockLayoutSerializer.h"
#include <JuceHeader.h>

/**
 * Simple manager for dock layout persistence
 * Handles automatic save/load with user preferences
 */
class DockLayoutManager
{
public:
    DockLayoutManager(JAdvancedDock& dock, const String& layoutName = "DefaultDockLayout")
        : dock(dock), layoutName(layoutName)
    {
        // Bestimme Speicherort
        auto appDataDir = File::getSpecialLocation(File::userApplicationDataDirectory);
        layoutFile = appDataDir.getChildFile("DockLayouts").getChildFile(layoutName + ".xml");

        // Erstelle Verzeichnis falls es nicht existiert
        layoutFile.getParentDirectory().createDirectory();
    }

    /**
     * Lade Layout beim Start der Anwendung
     * @return True wenn Layout erfolgreich geladen wurde
     */
    bool loadLayoutOnStartup()
    {
        if (!layoutFile.exists())
        {
            DBG("No saved layout found at: " << layoutFile.getFullPathName());
            return false;
        }

        bool success = JAdvancedDockLayoutSerializer::loadDockLayout(dock, layoutFile);

        if (success)
        {
            DBG("Layout loaded successfully from: " << layoutFile.getFullPathName());
            DBG(JAdvancedDockLayoutSerializer::getLayoutDescription(dock));
        }
        else
        {
            DBG("Failed to load layout from: " << layoutFile.getFullPathName());
        }

        return success;
    }

    /**
     * Speichere aktuelles Layout
     * @return True wenn erfolgreich gespeichert
     */
    bool saveCurrentLayout()
    {
        bool success = JAdvancedDockLayoutSerializer::saveDockLayout(dock, layoutFile);

        if (success)
        {
            DBG("Layout saved successfully to: " << layoutFile.getFullPathName());
            DBG(JAdvancedDockLayoutSerializer::getLayoutDescription(dock));
        }
        else
        {
            DBG("Failed to save layout to: " << layoutFile.getFullPathName());
        }

        return success;
    }

    /**
     * Automatisches Speichern beim Shutdown (für Timer oder ApplicationCommandTarget)
     */
    void saveLayoutOnShutdown()
    {
        saveCurrentLayout();
    }

    /**
     * Setze Layout auf Standardwerte zurück
     */
    void resetToDefault()
    {
        // Erstelle Standard-Layout
        JAdvancedDockLayoutSerializer::DockSizes defaultSizes;

        // Beispiel: 2 Zeilen mit verschiedenen Spalten
        defaultSizes.addRow(300.0); // Erste Zeile, 300px hoch
        defaultSizes.getRow(0).addColumn(250.0); // Erste Spalte: 250px breit
        defaultSizes.getRow(0).addColumn(400.0); // Zweite Spalte: 400px breit
        defaultSizes.getRow(0).addColumn(300.0); // Dritte Spalte: 300px breit

        defaultSizes.addRow(200.0); // Zweite Zeile, 200px hoch  
        defaultSizes.getRow(1).addColumn(500.0); // Eine breite Spalte: 500px
        defaultSizes.getRow(1).addColumn(450.0); // Zweite Spalte: 450px

        // Wende Standard-Größen an (nur wenn Struktur passt)
        if (JAdvancedDockLayoutSerializer::validateStructure(dock, defaultSizes))
        {
            JAdvancedDockLayoutSerializer::applySizes(dock, defaultSizes);
            DBG("Applied default layout sizes");
        }
        else
        {
            DBG("Cannot apply default sizes - dock structure doesn't match");
        }
    }

    /**
     * Exportiere Layout in andere Datei
     * @param exportFile Zieldatei für Export
     * @return True wenn erfolgreich
     */
    bool exportLayout(const File& exportFile)
    {
        return JAdvancedDockLayoutSerializer::saveDockLayout(dock, exportFile);
    }

    /**
     * Importiere Layout aus Datei
     * @param importFile Quelldatei für Import
     * @return True wenn erfolgreich
     */
    bool importLayout(const File& importFile)
    {
        return JAdvancedDockLayoutSerializer::loadDockLayout(dock, importFile);
    }

    /**
     * Prüfe ob gespeichertes Layout existiert
     * @return True wenn Layout-Datei existiert
     */
    bool hasSavedLayout() const
    {
        return layoutFile.exists();
    }

    /**
     * Hole aktueller Layout-Beschreibung für Debug/Info
     * @return String mit Layout-Information
     */
    String getCurrentLayoutInfo() const
    {
        return JAdvancedDockLayoutSerializer::getLayoutDescription(dock);
    }

    /**
     * Hole Pfad zur Layout-Datei
     * @return File-Objekt der Layout-Datei
     */
    File getLayoutFile() const
    {
        return layoutFile;
    }

private:
    JAdvancedDock& dock;
    String layoutName;
    File layoutFile;
};

/*
Verwendung in der Hauptkomponente:

class MainComponent : public Component
{
private:
    JAdvancedDock dock;
    DockLayoutManager layoutManager;

public:
    MainComponent() : dock(dockManager), layoutManager(dock, "MyAppLayout")
    {
        // Layout beim Start laden
        layoutManager.loadLayoutOnStartup();
    }

    ~MainComponent()
    {
        // Layout beim Beenden speichern
        layoutManager.saveLayoutOnShutdown();
    }

    void someUserAction()
    {
        // Manuell speichern wenn gewünscht
        layoutManager.saveCurrentLayout();
    }

    void resetLayout()
    {
        layoutManager.resetToDefault();
    }
};
*/