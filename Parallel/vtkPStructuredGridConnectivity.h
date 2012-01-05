/*=========================================================================

 Program:   Visualization Toolkit
 Module:    vtkPStructuredGridConnectivity.h

 Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 All rights reserved.
 See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
// .NAME vtkPStructuredGridConnectivity.h -- Constructs grid connectivity
//
// .SECTION Description
//  vtkPStructuredGridConnectivity inherits from vtkStructuredGridConnectivity
//  and implements functionality to compute the neighboring topology within a
//  single, partitioned and distributed structured grid dataset.
//
// .SECTION Caveats
//  Initialize(), ComputeNeighbors() and CreateGhostLayers() are collective
//  operations, every process must call that method.
//
// .SECTION See Also
//  vtkStructuredGridConnectivity vtkGhostArray

#ifndef VTKPSTRUCTUREDGRIDCONNECTIVITY_H_
#define VTKPSTRUCTUREDGRIDCONNECTIVITY_H_

// VTK include directives
#include "vtkStructuredGridConnectivity.h"

// C++ include directives
#include <vector> // For STL vector

// Forward declarations
class vtkMultiProcessController;

class VTK_PARALLEL_EXPORT vtkPStructuredGridConnectivity :
  public vtkStructuredGridConnectivity
{
  public:
    static vtkPStructuredGridConnectivity* New();
    vtkTypeMacro(vtkPStructuredGridConnectivity,vtkStructuredGridConnectivity);
    void PrintSelf(ostream& os, vtkIndent indent );

    // Description:
    // Set & Get the process controller
    vtkSetMacro( Controller,vtkMultiProcessController* );
    vtkGetMacro( Controller,vtkMultiProcessController* );

    // Description:
    // Sets the total number of domains distributed among processors
    void SetNumberOfGrids( const unsigned int N );

    // Description:
    // See vtkStructuredGridConnectivity::RegisterGrid
    void RegisterGrid( const int gridID, int extents[6],
        vtkUnsignedCharArray* nodesGhostArray,
        vtkUnsignedCharArray* cellGhostArray,
        vtkPointData* pointData,
        vtkCellData* cellData,
        vtkPoints* gridNodes );

    // Description:
    // Returns the number of local grids registers by the process that owns
    // the current vtkPStructuredGridConnectivity instance
    int GetNumberOfLocalGrids() { return this->GridIds.size(); };

    // Description:
    // Returns the rank of the given gridID. A nominal value of -1 for the
    // return value of this method indicates that possibly ComputeNeighbors
    // has not been called and consequently the GridRanks vector has not been
    // populated yet.
    int GetGridRank( const int gridID );

    // Description:
    // Returns true iff the grid corresponding to the given gridID is local.
    bool IsGridLocal( const int gridID );

    // Description:
    // Initializes this instance of vtkPStructuredGridConnectivity, essentially,
    // the acquires the local process ID from the registered controller. If a
    // controller is not registered, the global controller is set.
    void Initialize();

    // Description:
    // Computes the neighboring topology of a distributed structured grid dataset.
    // See vtkStructuredGridConnectivity::ComputeNeighbors
    void ComputeNeighbors();

    // Description:
    // Creates ghost layers on the grids owned by this process using data from
    // both local and remote block neighbors.
    virtual void CreateGhostLayers( const int N=1 );

  protected:
    vtkPStructuredGridConnectivity();
    ~vtkPStructuredGridConnectivity();

    vtkMultiProcessController *Controller;
    int Rank;
    bool Initialized;

    // BTX
    std::vector< int > GridRanks;
    std::vector< int > GridIds;
    std::vector< vtkPointData* > SendData;
    std::vector< vtkCellData* >  RcvData;
    // ETX

    // Description:
    // Registers a remote grid with the given grid Id, structured extents and
    // process.
    void RegisterRemoteGrid( const int gridID, int extents[6], int process );

    // Description:
    // Exchanges ghost data of the grids owned by this process
    void ExchangeGhostData();

    // Description:
    // Exchanges the grid extents among all processes and fully populates the
    // GridExtents vector.
    void ExchangeGridExtents();

    // Description:
    // Serializes the grid extentts and information in a buffer to send over MPI
    // The data is serialized as follows: ID imin imax jmin jmax kmin kmax
    void SerializeGridExtents( int *&sndbuffer, vtkIdType &N );

    // Description:
    // Deserializes the received grid extent information to the GridExtents
    // internal data-structures.
    void DeserializeGridExtentForProcess(
        int *rcvbuffer, vtkIdType &N, const int processId );

  private:
    vtkPStructuredGridConnectivity(const vtkPStructuredGridConnectivity& ); // Not implemented
    void operator=(const vtkPStructuredGridConnectivity& ); // Not implemented
};

//=============================================================================
//  INLINE METHODS
//=============================================================================

inline bool vtkPStructuredGridConnectivity::IsGridLocal(const int gridID)
{
  assert( "pre: Instance has not been intialized!" && this->Initialized );
  assert( "pre: gridID is out-of-bounds" &&
          (gridID >= 0) && (gridID < static_cast<int>(this->NumberOfGrids) ) );
  assert( "pre: GridRanks is not properly allocated" &&
          this->NumberOfGrids == this->GridRanks.size() );
  return( (this->GridRanks[ gridID ] == this->Rank) );
}

//------------------------------------------------------------------------------
inline int vtkPStructuredGridConnectivity::GetGridRank( const int gridID )
{
  assert( "pre: Instance has not been intialized!" && this->Initialized );
  assert( "pre: gridID out-of-bounds!" &&
          (gridID >= 0  && gridID < static_cast<int>(this->NumberOfGrids)));
  return( this->GridRanks[ gridID ] );
}
#endif /* VTKPSTRUCTUREDGRIDCONNECTIVITY_H_ */
