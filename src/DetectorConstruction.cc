//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: DetectorConstruction.cc, 2016-03-29 $
// GEANT4 tag $Name: geant4.10.02.p01 $
// Developer: Chao Peng
//
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "DetectorConstruction.hh"

#include "Digitization.hh"
#include "RootTree.hh"
#include "CalorimeterSD.hh"
#include "GasElectronMultiplierSD.hh"
#include "VirtualDetectorSD.hh"
#include "DetectorMessenger.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4SDManager.hh"
#include "G4VSensitiveDetector.hh"

#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Polycone.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4PVParameterised.hh"

#include "G4String.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4RunManager.hh"

#include <cmath>
#include <map>
#include <vector>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction() : daq_system(NULL), otree(NULL)
{
    detectorMessenger = new DetectorMessenger(this);

    daq_system = new Digitization();
    otree = new RootTree();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{
    delete daq_system;
    delete otree;

    delete detectorMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume *DetectorConstruction::Construct()
{
    // Material
    std::map<G4String, G4VisAttributes *> mVisAtt;
    
    G4int z;
    G4double a;
    G4double density;
    G4int ncomponents, natoms;
    G4double fractionmass;

    G4NistManager *pNM = G4NistManager::Instance();

    // Define elements
    G4Element *H  = pNM->FindOrBuildElement(z = 1);
    G4Element *C  = pNM->FindOrBuildElement(z = 6);
    G4Element *N  = pNM->FindOrBuildElement(z = 7);
    G4Element *O  = pNM->FindOrBuildElement(z = 8);
    G4Element *Al = pNM->FindOrBuildElement(z = 13);
    G4Element *Si = pNM->FindOrBuildElement(z = 14);
    G4Element *Ar = pNM->FindOrBuildElement(z = 18);
    G4Element *Cu = pNM->FindOrBuildElement(z = 29);
    G4Element *W  = pNM->FindOrBuildElement(z = 74);
    G4Element *Pb = pNM->FindOrBuildElement(z = 86);

    // Space Vacuum
    G4Material *Galaxy = new G4Material("Galaxy", z = 1, a = 1.01 * g / mole, density = universe_mean_density, kStateGas, 0.1 * kelvin, 1.0e-19 * pascal);

    // Air
    G4Material *Air = new G4Material("Air", density = 1.29 * mg / cm3, ncomponents = 2);
    Air->AddElement(N, fractionmass = 0.7);
    Air->AddElement(O, fractionmass = 0.3);

    // Air vacuum of 1.e-6 torr at room temperature, 1 atmosphere = 760 torr
    G4Material *Vacuum = new G4Material("Vacuum", density = 1.0e-6 / 760.0 * 1.225 * mg / cm3, ncomponents = 1, kStateGas, STP_Temperature, 1.0e-6 / 760.0 * atmosphere);
    Vacuum->AddMaterial(Air, fractionmass = 1.0);

    // Hydrogen Gas
    G4Material *H2Gas =  new G4Material("H2 Gas", density = 8.988e-5 * g / cm3, ncomponents = 1, kStateGas, 25.0 * kelvin, 83.02 * pascal);
    H2Gas->AddElement(H, natoms = 2);
    
    // Copper
    G4Material *Copper = new G4Material("Copper", density = 8.96 * g / cm3, ncomponents = 1);
    Copper->AddElement(Cu, natoms = 1);

    // Kapton
    G4Material *Kapton = new G4Material("Kapton", density = 1.42 * g / cm3, ncomponents = 4);
    Kapton->AddElement(H, fractionmass = 0.0273);
    Kapton->AddElement(C, fractionmass = 0.7213);
    Kapton->AddElement(N, fractionmass = 0.0765);
    Kapton->AddElement(O, fractionmass = 0.1749);

    // Aluminum
    G4Material *Aluminum = new G4Material("Aluminum", density = 2.700 * g / cm3, ncomponents = 1);
    Aluminum->AddElement(Al, natoms = 1);

    // GEM Frame G10
    G4Material *NemaG10 = new G4Material("NemaG10", density = 1.700 * g / cm3, ncomponents = 4);
    NemaG10->AddElement(Si, natoms = 1);
    NemaG10->AddElement(O , natoms = 2);
    NemaG10->AddElement(C , natoms = 3);
    NemaG10->AddElement(H , natoms = 3);

    // CO2 Gas
    G4Material *CO2 = new G4Material("CO2", density = 1.842e-3 * g / cm3, ncomponents = 2);
    CO2->AddElement(C, natoms = 1);
    CO2->AddElement(O, natoms = 2);

    // Ar/CO2 Gas
    G4Material *ArCO2 = new G4Material("ArCO2", density = 1.715e-3 * g / cm3, ncomponents = 2);
    ArCO2->AddElement(Ar, fractionmass = 0.7);
    ArCO2->AddMaterial(CO2, fractionmass = 0.3);

    // Torlon4203L
    G4Material *Torlon = new G4Material("Torlon", density = 1.412 * g / cm3, ncomponents = 5);
    Torlon->AddElement(C , natoms = 9);
    Torlon->AddElement(H , natoms = 4);
    Torlon->AddElement(N , natoms = 2);
    Torlon->AddElement(O , natoms = 3);
    Torlon->AddElement(Ar, natoms = 1);

    // Tungsten
    G4Material *Tungsten = new G4Material("Tungsten", density = 19.25 * g / cm3, ncomponents = 1);
    Tungsten->AddElement(W, natoms = 1);

    // PbWO4 Crystal
    G4Material *PbWO4 = new G4Material("PbWO4", density = 8.300 * g / cm3, ncomponents = 3);
    PbWO4->AddElement(Pb, natoms = 1);
    PbWO4->AddElement(W , natoms = 1);
    PbWO4->AddElement(O , natoms = 4);

    // Lead Glass
    G4Material *SiO2 = new G4Material("Quartz", density = 2.200 * g / cm3, ncomponents = 2);
    SiO2->AddElement(Si, natoms = 1);
    SiO2->AddElement(O , natoms = 2);
    G4Material *PbGlass = new G4Material("Lead Glass", density = 3.85 * g / cm3, ncomponents = 2);
    PbGlass->AddElement(Pb, fractionmass = 0.5316);
    PbGlass->AddMaterial(SiO2, fractionmass = 0.4684);

    // Print out material table
    G4cout << *(G4Material::GetMaterialTable()) << G4endl;

    mVisAtt[Vacuum->GetName()] = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 0.5)); // Vacuum;
    mVisAtt[H2Gas->GetName()] = new G4VisAttributes(G4Colour::Cyan());
    mVisAtt[Copper->GetName()] = new G4VisAttributes(G4Colour::Brown());
    mVisAtt[Kapton->GetName()] = new G4VisAttributes(G4Colour::Brown());
    mVisAtt[Aluminum->GetName()] = new G4VisAttributes(G4Colour::Grey());
    mVisAtt[NemaG10->GetName()] = new G4VisAttributes(G4Colour::Magenta());
    mVisAtt[ArCO2->GetName()] = new G4VisAttributes(G4Colour::Yellow());
    mVisAtt[Torlon->GetName()] = new G4VisAttributes(G4Colour::Grey());
    mVisAtt[Tungsten->GetName()] = new G4VisAttributes(G4Colour::Black());
    mVisAtt[PbWO4->GetName()] = new G4VisAttributes(G4Colour::Blue());
    mVisAtt[PbGlass->GetName()] = new G4VisAttributes(G4Colour::Blue());
    mVisAtt[Galaxy->GetName()] = new G4VisAttributes(G4VisAttributes::Invisible);
   
    G4Material *VacuumMaterial = Vacuum;
    G4Material *TarCelMaterial = Copper;
    G4Material *WindowsMaterial = Kapton;
    G4Material *ChamberMaterial = Aluminum;
    G4Material *GEMFrameMaterial = NemaG10;
    G4Material *GEMFoilMaterial = Kapton;
    G4Material *GEMGasMaterial = ArCO2;
    G4Material *HyCalBoxMaterial = Torlon;
    G4Material *CollimatorMaterial = Tungsten;
    G4Material *CenterHyCalMaterial = PbWO4;
    //G4Material *OuterHyCalMaterial = PbGlass;
    G4Material *defaultMaterial = Galaxy;
    
    TargetMaterial = H2Gas;

    // World
    G4double WorldSizeXY = 150.0 * cm;
    G4double WorldSizeZ = 400.0 * cm;
    G4VSolid *solidWorld = new G4Box("Solid World", WorldSizeXY, WorldSizeXY, WorldSizeZ);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, defaultMaterial, "Logical World");
    physiWorld = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicWorld, "World", 0, false, 0);

    // Target
    G4double TargetCenter = -300.0 * cm;
    G4VSolid *solidTargetCon = new G4Box("Solid Target Container", 3.5 * cm, 3.5 * cm, 2.1 * cm);
    G4LogicalVolume *logicTargetCon = new G4LogicalVolume(solidTargetCon, defaultMaterial, "Logical Target Container");
    new G4PVPlacement(0, G4ThreeVector(0, 0, TargetCenter), logicTargetCon, "Target Container", logicWorld, false, 0);

    // Target material
    G4double TargetR = 25.0 * mm;
    G4double TargetHalfL = 20.0 * mm;
    G4VSolid *solidTarget = new G4Tubs("Solid Target Material", 0, TargetR, TargetHalfL, 0, twopi);
    G4LogicalVolume *logicTarget = new G4LogicalVolume(solidTarget, TargetMaterial, "Logical Target Material");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicTarget, "Target Material", logicTargetCon, false, 0);

    // Target cell
    G4double CellXY = 3.5 * cm;
    G4Box *CellBox = new G4Box("Cell Box", CellXY, CellXY, TargetHalfL);
    G4Tubs *CellTube = new G4Tubs("Cell Tube", 0, TargetR, TargetHalfL + 1.0 * mm, 0, twopi);
    G4SubtractionSolid *solidCell = new G4SubtractionSolid("Solid Target Cell", CellBox, CellTube);
    G4LogicalVolume *logicCell = new G4LogicalVolume(solidCell, TarCelMaterial, "Logical Target Cell");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicCell, "Target Cell", logicTargetCon, false, 0);
    
    // Windows
    G4double CellApertureR = 2.0 * mm;
    G4double CellWinThickness = 7.5 * um;
    G4Box *CellWinBox = new G4Box("Cell Window Box", CellXY, CellXY, CellWinThickness / 2.0);
    G4Tubs *CellWinTube = new G4Tubs("Cell Window Tube", 0, CellApertureR, CellWinThickness + 1.0 * mm, 0, twopi);
    G4SubtractionSolid *solidCellWin = new G4SubtractionSolid("Solid Target Cell", CellWinBox, CellWinTube);
    G4LogicalVolume *logicCellWin = new G4LogicalVolume(solidCellWin, WindowsMaterial, "Logical Target Window");
    new G4PVPlacement(0, G4ThreeVector(0, 0, -TargetHalfL - CellWinThickness / 2.0), logicCellWin, "Upstream Target Window", logicTargetCon, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, +TargetHalfL + CellWinThickness / 2.0), logicCellWin, "Downstream Target Window", logicTargetCon, false, 0);

    // Target chamber
    G4double DownChamberCenter = -254.705 * cm;
    G4double DownChamberHalfL = 71.79 / 2.0 * cm;
    G4double DownChamberUR = 8.00 * cm;
    G4double DownChamberDR = 17.30 * cm;

    // Downstream chamber // For now, only built the downstream chamber with window
    G4double rInner1[] = {7.56 * cm, 7.56 * cm, 7.56 * cm, 7.56 * cm, 17.30 * cm, 17.30 * cm};
    G4double rOuter1[] = {8.00 * cm, 8.00 * cm, 17.78 * cm, 17.78 * cm, 17.78 * cm, 17.78 * cm};
    G4double zPlane1[] = {0, 33.62 * cm, 33.62 * cm, 36.16 * cm, 36.16 * cm, 71.79 * cm};
    G4VSolid *solidDownChamber = new G4Polycone("Solid Downstream Chamber", 0, twopi, 6, zPlane1, rInner1, rOuter1);
    G4LogicalVolume *logicDownChamber = new G4LogicalVolume(solidDownChamber, ChamberMaterial, "Logical Downstream Chamber");
    new G4PVPlacement(0, G4ThreeVector(0, 0, DownChamberCenter - DownChamberHalfL), logicDownChamber, "Downstream Chamber", logicWorld, false, 0);
    
    // Windows
    G4double DownChamberApertureR = 22.8 * mm;
    G4double DownChamberWinThickness = 7.5 * um;
    G4double DownChamberWinOff = 25.9 * mm;
    G4Tubs *solidDownChamberWin1 = new G4Tubs("Solid Downstream Chamber Window 1", DownChamberApertureR, DownChamberUR, DownChamberWinThickness / 2.0, 0, twopi);
    G4Tubs *solidDownChamberWin2 = new G4Tubs("Solid Downstream Chamber Window 2", DownChamberApertureR, DownChamberDR, DownChamberWinThickness / 2.0, 0, twopi);
    G4LogicalVolume *logicDownChamberWin1 = new G4LogicalVolume(solidDownChamberWin1, WindowsMaterial, "Logical Downstream Chamber Window 1");
    G4LogicalVolume *logicDownChamberWin2 = new G4LogicalVolume(solidDownChamberWin2, WindowsMaterial, "Logical Downstream Chamber Window 2");
    new G4PVPlacement(0, G4ThreeVector(0, 0, DownChamberCenter - DownChamberHalfL - DownChamberWinThickness / 2.0), logicDownChamberWin1, "Downstream Chamber Window", logicWorld, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, DownChamberCenter + DownChamberHalfL - DownChamberWinOff), logicDownChamberWin2, "Downstream Chamber Window", logicWorld, false, 0);

    // Vacuum box
    G4double VacBoxCenter = -6.31 * cm;
    G4double VacBoxHalfL = 212.5 * cm;
    G4double VacBoxMaxR = 78.11 * cm;
    G4double rInner2[] = {17.30 * cm, 17.30 * cm, 50.17 * cm, 50.17 * cm, 78.11 * cm, 78.11 * cm};
    G4double rOuter2[] = {17.78 * cm, 17.78 * cm, 50.80 * cm, 50.80 * cm, 78.74 * cm, 78.74 * cm};
    G4double zPlane2[] = {0, 6.8 * cm, 17.6 * cm, 215.3 * cm, 229.5 * cm, 425.00 * cm};
    G4VSolid *solidVacBox = new G4Polycone("Solid Vacuum Box", 0, twopi, 6, zPlane2, rInner2, rOuter2);
    G4LogicalVolume *logicVacBox = new G4LogicalVolume(solidVacBox, ChamberMaterial, "Logical Vacuum Box");
    new G4PVPlacement(0, G4ThreeVector(0, 0, VacBoxCenter - VacBoxHalfL), logicVacBox, "Vacuum Box", logicWorld, false, 0);

    // Window and flange
    G4double ArcDistance = 5.59 * cm;
    G4double ArcEndR = (ArcDistance * ArcDistance + VacBoxMaxR * VacBoxMaxR) / (2 * ArcDistance);
    G4double ArcEndThickness = 1.6 * mm;
    G4double FlangeIR = 1.9 * cm;
    G4double FlangeOR = 3.0 * cm;
    G4double FlangeHalfL = 0.5 * cm;
    G4Sphere *VacSphere = new G4Sphere("Vac Sphere", ArcEndR - ArcEndThickness, ArcEndR, 0, twopi, pi - asin(VacBoxMaxR / ArcEndR), pi);
    G4Tubs *VacHole = new G4Tubs("Vac Hole", 0, FlangeOR, ArcEndR + 1.0 * mm, 0, twopi);
    G4SubtractionSolid *solidVacBoxWin = new G4SubtractionSolid("Solid Vacuum Box Window", VacSphere, VacHole);
    G4LogicalVolume *logicVacBoxWin = new G4LogicalVolume(solidVacBoxWin, ChamberMaterial, "Logical Vacuum Box Window");
    new G4PVPlacement(0, G4ThreeVector(0, 0, VacBoxCenter + VacBoxHalfL + ArcEndR - ArcDistance), logicVacBoxWin, "Vacuum Box Window", logicWorld, false, 0);
    G4VSolid *solidFlange = new G4Tubs("Solid Vacuum Flange", FlangeIR, FlangeOR, FlangeHalfL, 0, twopi);
    G4LogicalVolume *logicFlange = new G4LogicalVolume(solidFlange, ChamberMaterial, "Logical Vacuum Flange");
    new G4PVPlacement(0, G4ThreeVector(0, 0, VacBoxCenter + VacBoxHalfL - ArcDistance + FlangeHalfL), logicFlange, "Vacuum Flange", logicWorld, false, 0);

    // GEM
    G4double GEMCenter = 220.25 * cm; // Center of two GEM // (522.2 + 518.3) / 2 - 300.0 from Weizhi
    G4double GEMGap = 4.0 * cm; // Gap between two GEM
    G4RotationMatrix rmGEM2;
    rmGEM2.rotateZ(180.0 * deg);
    G4Box *GEMOuterFrame = new G4Box("GEM OuterFrame", 332.5 * mm, 699.9 * mm, 6.0 * mm);
    G4Tubs *GEMPipeHole = new G4Tubs("GEM Pipe Hole", 0, 22.0 * mm, 6.1 * mm, 0, twopi);
    G4SubtractionSolid *solidGEMCon = new G4SubtractionSolid("Solid GEM Container", GEMOuterFrame, GEMPipeHole, 0, G4ThreeVector(-253.0 * mm, 0, 0));
    G4LogicalVolume *logicGEMCon = new G4LogicalVolume(solidGEMCon, defaultMaterial, "Logical GEM Container");
    new G4PVPlacement(0, G4ThreeVector(25.3 * cm, 0, GEMCenter - GEMGap / 2.0), logicGEMCon, "GEM Container", logicWorld, false, 0);
    new G4PVPlacement(G4Transform3D(rmGEM2, G4ThreeVector(-25.3 * cm, 0, GEMCenter + GEMGap / 2.0)), logicGEMCon, "GEM Container", logicWorld, false, 0);

    // GEM Gas
    G4Box *GEMGasPiece1 = new G4Box("GEM Gas Piece 1", 275.0 * mm, 674.4 * mm, 6.0 * mm);
    G4Box *GEMGasPiece2 = new G4Box("GEM Gas Piece 2", 37.0 * mm, 29.5 * mm, 6.2 * mm);
    G4SubtractionSolid *solidGEMGas = new G4SubtractionSolid("Solid GEM Gas", GEMGasPiece1, GEMGasPiece2, 0, G4ThreeVector(-245.5 * mm, 0, 0));
    G4LogicalVolume *logicGEMGas = new G4LogicalVolume(solidGEMGas, GEMGasMaterial, "Logical GEM Gas");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicGEMGas, "GEM Gas", logicGEMCon, false, 0);

    // GEM Frame
    G4Box *GEMFramePiece1 = new G4Box("GEM Frame Piece 1", 275.0 * mm, 674.4 * mm, 6.1 * mm);
    G4SubtractionSolid *GEMFramePiece2 = new G4SubtractionSolid("GEM Frame Piece 2", GEMFramePiece1, GEMGasPiece2, 0, G4ThreeVector(-245.5 * mm, 0, 0));
    G4SubtractionSolid *GEMFrameNoHole = new G4SubtractionSolid("GEM Frame No Hole", GEMOuterFrame, GEMFramePiece2);
    G4SubtractionSolid *solidGEMFrame = new G4SubtractionSolid("Solid GEM Frame", GEMFrameNoHole, GEMPipeHole, 0, G4ThreeVector(-253.0 * mm, 0, 0));
    G4LogicalVolume *logicGEMFrame = new G4LogicalVolume(solidGEMFrame, GEMFrameMaterial, "Logical GEM Frame");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicGEMFrame, "GEM Frame", logicGEMCon, false, 0);

    // GEM Foil
    G4double GEMFoilThickness = 55.0 * um;
    G4double GEMCoverThickness = 25.0 * um;
    G4Box *GEMCoverBox = new G4Box("GEM Cover Box", 275.0 * mm, 674.4 * mm, GEMCoverThickness / 2.0);
    G4SubtractionSolid *solidGEMCover = new G4SubtractionSolid("Solid GEM Cover", GEMCoverBox, GEMGasPiece2, 0, G4ThreeVector(-245.5 * mm, 0, 0));
    G4LogicalVolume *logicGEMCover = new G4LogicalVolume(solidGEMCover, GEMFoilMaterial, "Logical GEM Cover");
    G4Box *GEMFoilBox = new G4Box("GEM Foil Box", 275.0 * mm, 674.4 * mm, GEMFoilThickness / 2.0);
    G4SubtractionSolid *solidGEMFoil = new G4SubtractionSolid("Solid GEM Foil", GEMFoilBox, GEMGasPiece2, 0, G4ThreeVector(-245.5 * mm, 0, 0));
    G4LogicalVolume *logicGEMFoil = new G4LogicalVolume(solidGEMFoil, GEMFoilMaterial, "Logical GEM Foil");
    G4LogicalVolume *logicGEMReadout = new G4LogicalVolume(solidGEMFoil, GEMFoilMaterial, "Logical GEM Readout");
    new G4PVPlacement(0, G4ThreeVector(0, 0, -6.0 * mm + GEMCoverThickness / 2.0), logicGEMCover, "GEM Cover Foil", logicGEMGas, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, -3.0 * mm), logicGEMFoil, "GEM Foil", logicGEMGas, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicGEMFoil, "GEM Foil", logicGEMGas, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, 2.0 * mm), logicGEMFoil, "GEM Foil", logicGEMGas, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, 4.0 * mm), logicGEMReadout, "GEM Readout Foil", logicGEMGas, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, 6.0 * mm - GEMCoverThickness / 2.0), logicGEMCover, "GEM CoverFoil", logicGEMGas, false, 0);

    // HyCal
    G4double PbGlassL = 45.0 * cm;
    //G4double CrystalL = 18.0 * cm;
    G4double CrystalDiffL = 10.12 * cm;
    G4double CrystalSurfCenter = 264.0 * cm; // Surface of the PWO // 564.0 - 300.0 from Weizhi
    G4double HyCalCenter = CrystalSurfCenter - CrystalDiffL + PbGlassL / 2.0; 
    G4double HyCalBoxHalfL = 60.0 * cm;

    // HyCal box
    G4double HyCalBoxCenter = HyCalCenter - 9.0 * cm + 30.0 * cm; // Check
    G4Box *HyCalBoxOuter = new G4Box("HyCal Box Outer", 70.0 * cm, 70.0 * cm, 60.0 * cm);
    G4Box *HyCalBoxInner = new G4Box("HyCal Box Inner", 66.0 * cm, 66.0 * cm, 59.6 * cm);
    G4SubtractionSolid *HyCalBoxNoHole = new G4SubtractionSolid("HyCal Box No Hole", HyCalBoxOuter, HyCalBoxInner);
    G4Tubs *HyCalBoxHole = new G4Tubs("HyCal Box Hole", 0, 25.0 * mm, 60.5 * cm, 0, twopi);
    G4SubtractionSolid *solidHyCalBox = new G4SubtractionSolid("Solid HyCal Box", HyCalBoxNoHole, HyCalBoxHole);
    G4LogicalVolume *logicHyCalBox = new G4LogicalVolume(solidHyCalBox, HyCalBoxMaterial, "Logical HyCal Box");
    new G4PVPlacement(0, G4ThreeVector(0, 0, HyCalBoxCenter), logicHyCalBox, "HyCal Box", logicWorld, false, 0);

    // HyCal container
    G4Box *HyCalConPiece1 = new G4Box("HyCal Container Piece 1", 58.21 * cm, 58.17 * cm, PbGlassL / 2.0);
    G4Box *HyCalConPiece2 = new G4Box("HyCal Container Piece 2", 35.30 * cm, 35.27 * cm, CrystalDiffL / 2.0 + 0.5 * mm);
    G4SubtractionSolid *HyCalConBox = new G4SubtractionSolid("HyCal Container Box", HyCalConPiece1, HyCalConPiece2, 0, G4ThreeVector(0, 0, (CrystalDiffL - PbGlassL) / 2.0 - 0.5 * mm));
    G4Box *HyCalConHole = new G4Box("HyCal Container Hole", 2.0 * cm, 2.0 * cm, PbGlassL / 2.0 + 1.0 * mm);
    G4SubtractionSolid *solidHyCalCon = new G4SubtractionSolid("Solid HyCal Container", HyCalConBox, HyCalConHole);
    G4LogicalVolume *logicHyCalCon = new G4LogicalVolume(solidHyCalCon, defaultMaterial, "Logical HyCal Container");
    new G4PVPlacement(0, G4ThreeVector(0, 0, HyCalCenter), logicHyCalCon, "HyCal Container", logicWorld, false, 0);

    // HyCal modules
    G4VSolid *solidAbsorber = new G4Box("Solid Crystal Block", 1.025 * cm, 1.025 * cm, 90.0 * mm);
    G4LogicalVolume *logicAbsorber = new G4LogicalVolume(solidAbsorber, CenterHyCalMaterial, "Logical Crystal Block");
    HyCalParameterisation *param = new HyCalParameterisation("config/module_list.txt",  // load modules
            "config/pedestal.dat",     // set module pedestals
            "config/calibration.txt"); // set module calibration factors
    new G4PVParameterised("HyCal Crystal", logicAbsorber, logicHyCalCon, kUndefined, param->GetNumber(), param, false);

    // Collimators around the central hole
    G4VSolid *CollConBox = new G4Box("HyCal Collimator Container Box", 4.1 * cm, 4.1 * cm, 5.0 * cm);
    G4SubtractionSolid *solidCollCon = new G4SubtractionSolid("Solid HyCal Collimator Container", CollConBox, HyCalConHole);
    G4LogicalVolume *logicCollCon = new G4LogicalVolume(solidCollCon, defaultMaterial, "Logical HyCal Collimator Container");
    new G4PVPlacement(0, G4ThreeVector(0, 0, HyCalCenter - PbGlassL / 2.0 + CrystalDiffL - 5.1 * cm), logicCollCon, "HyCal Collimator Container", logicWorld, false, 0);
    G4VSolid *solidColl = new G4Box("Solid HyCal Collimator", 1.025 * cm, 1.025 * cm, 5.0 * cm);
    G4LogicalVolume *logicColl = new G4LogicalVolume(solidColl, CollimatorMaterial, "Logical HyCal Collimator");
    double pos_x[12] = { -3.075, -1.025, 1.025, 3.075, -3.075, 3.075, -3.075, 3.075, -3.075, -1.025, 1.025, 3.075};
    double pos_y[12] = { -3.075, -3.075, -3.075, -3.075, -1.025, -1.025, 1.025, 1.025, 3.075, 3.075, 3.075, 3.075};

    for (int i = 0; i < 12; ++i)
        new G4PVPlacement(0, G4ThreeVector(pos_x[i] * cm, pos_y[i] * cm, 0), logicColl, "HyCal Collimator", logicCollCon, false, 0);

    // Vacuum Tube
    G4double VacTubeIR = 1.8 * cm;
    G4double VacTubeOR = 1.9 * cm;
    G4double VacTubeL = HyCalBoxHalfL + HyCalBoxCenter - VacBoxCenter - VacBoxHalfL + ArcDistance;
    G4VSolid *solidVacTube = new G4Tubs("Solid Vacuum Tube", VacTubeIR, VacTubeOR, VacTubeL / 2.0, 0, twopi);
    G4LogicalVolume *logicVacTube = new G4LogicalVolume(solidVacTube, ChamberMaterial, "Logical Vacuum Tube");
    new G4PVPlacement(0, G4ThreeVector(0, 0, HyCalBoxCenter + HyCalBoxHalfL - VacTubeL / 2.0), logicVacTube, "Vacuum Tube", logicWorld, false, 0);

    // Virtual Detector
    G4RotationMatrix rmVD;
    rmVD.rotateZ(90.0 * deg);
    G4Box *VDPiece1 = new G4Box("Virtual Detector Piece 1", 58.21 * cm, 58.17 * cm, 0.5 * mm);
    G4Box *VDPiece2 = new G4Box("Virtual Detector Piece 2", 35.30 * cm, 35.27 * cm, 0.6 * mm);
    G4SubtractionSolid *VDPiece3 = new G4SubtractionSolid("Virtual Detector Piece 3", VDPiece1, VDPiece2);
    G4Box *VDPiece4 = new G4Box("Virtual Detector Piece 4", 1.97 * cm / 2.0, 22.86 * cm / 2.0, 0.6 * mm);
    G4SubtractionSolid *VDPiece5 = new G4SubtractionSolid("Virtual Detector Piece 5", VDPiece3, VDPiece4, 0, G4ThreeVector((58.21 - 1.97 / 2.0 + 0.001) * cm, (58.17 - 22.86 / 2.0 + 0.001) * cm, 0));
    G4SubtractionSolid *VDPiece6 = new G4SubtractionSolid("Virtual Detector Piece 6", VDPiece5, VDPiece4, G4Transform3D(rmVD, G4ThreeVector(-(58.21 - 22.86 / 2.0 + 0.001) * cm, (58.17 - 1.97 / 2.0 + 0.001) * cm, 0)));
    G4SubtractionSolid *VDPiece7 = new G4SubtractionSolid("Virtual Detector Piece 7", VDPiece6, VDPiece4, 0, G4ThreeVector(-(58.21 - 1.97 / 2.0 + 0.001) * cm, -(58.17 - 22.86 / 2.0 + 0.001) * cm, 0));
    G4SubtractionSolid *solidVD1 = new G4SubtractionSolid("Solid Virtual Detector 1", VDPiece7, VDPiece4, G4Transform3D(rmVD, G4ThreeVector((58.21 - 22.86 / 2.0 + 0.001) * cm, -(58.17 - 1.97 / 2.0 + 0.001) * cm, 0)));
    G4Box *VDHole = new G4Box("Virtual Detector Hole", 2.075 * cm, 2.075 * cm, 0.6 * mm);
    G4Box *VDPiece10 = new G4Box("Virtual Detector Piece 10", 35.30 * cm, 35.27 * cm, 0.5 * mm);
    G4SubtractionSolid *solidVD2 = new G4SubtractionSolid("Solid Virtual Detector 2", VDPiece10, VDHole);
    G4LogicalVolume *logicVD1 = new G4LogicalVolume(solidVD1, VacuumMaterial, "Logical Virtual Detector 1");
    new G4PVPlacement(0, G4ThreeVector(0, 0, HyCalCenter - PbGlassL / 2.0 - 0.5 * mm), logicVD1, "Virtual Detector", logicWorld, false, 0);
    G4LogicalVolume *logicVD2 = new G4LogicalVolume(solidVD2, VacuumMaterial, "Logical Virtual Detector 2");
    new G4PVPlacement(0, G4ThreeVector(0, 0, HyCalCenter - PbGlassL / 2.0 + CrystalDiffL - 0.5 * mm), logicVD2, "Virtual Detector", logicWorld, false, 0);

    // Sensitive detectors
    G4SDManager *SDman = G4SDManager::GetSDMpointer();
    daq_system->RegisterModules(param);
    CalorimeterSD *HyCalSD = new CalorimeterSD("pradsim/CalorimeterSD", daq_system);
    GasElectronMultiplierSD *GEMSD = new GasElectronMultiplierSD("pradsim/GasElectronMultiplierSD", daq_system);
    VirtualDetectorSD *VirtualSD = new VirtualDetectorSD("pradsim/VirtualDetectorSD", otree, daq_system);
    SDman->AddNewDetector(HyCalSD);
    SDman->AddNewDetector(GEMSD);
    SDman->AddNewDetector(VirtualSD);
    logicAbsorber->SetSensitiveDetector(HyCalSD);
    logicGEMReadout->SetSensitiveDetector(GEMSD);
    logicVD1->SetSensitiveDetector(VirtualSD);
    logicVD2->SetSensitiveDetector(VirtualSD);

    G4LogicalVolumeStore *pLogicalVolume = G4LogicalVolumeStore::GetInstance();

    for (unsigned long i = 0; i < pLogicalVolume->size(); i++)
        (*pLogicalVolume)[i]->SetVisAttributes(mVisAtt[(*pLogicalVolume)[i]->GetMaterial()->GetName()]);

    // Always return the physical World
    return physiWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetTargetMaterial(G4String materialChoice)
{
    G4Material *pttoMaterial = G4Material::GetMaterial(materialChoice);

    if (pttoMaterial) TargetMaterial = pttoMaterial;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::UpdateGeometry()
{
    G4RunManager::GetRunManager()->DefineWorldVolume(Construct());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
