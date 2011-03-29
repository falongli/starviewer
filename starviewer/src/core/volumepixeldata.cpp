#include "volumepixeldata.h"

#include <vtkImageData.h>
// Voxel information
#include <vtkPointData.h>
#include <vtkCell.h>

#include "logging.h"

namespace udg {

VolumePixelData::VolumePixelData(QObject *parent) :
    QObject(parent)
{
    m_imageDataVTK = vtkSmartPointer<vtkImageData>::New();

    m_itkToVtkFilter = ItkToVtkFilterType::New();
    m_vtkToItkFilter = VtkToItkFilterType::New();
}

VolumePixelData::ItkImageTypePointer VolumePixelData::getItkData()
{
    m_vtkToItkFilter->SetInput(this->getVtkData());
    try
    {
        m_vtkToItkFilter->GetImporter()->Update();
    }
    catch (itk::ExceptionObject &excep)
    {
        WARN_LOG(QString("Excepció en el filtre vtkToItk :: Volume::getItkData() -> ") + excep.GetDescription());
    }
    return m_vtkToItkFilter->GetImporter()->GetOutput();
}

void VolumePixelData::setData(ItkImageTypePointer itkImage)
{
    m_itkToVtkFilter->SetInput(itkImage);
    try
    {
        m_itkToVtkFilter->Update();
    }
    catch (itk::ExceptionObject & excep)
    {
        WARN_LOG(QString("Excepció en el filtre itkToVtk :: Volume::setData(ItkImageTypePointer itkImage) -> ") + excep.GetDescription());
    }
    this->setData(m_itkToVtkFilter->GetOutput());
}

vtkImageData* VolumePixelData::getVtkData()
{
    return m_imageDataVTK;
}

void VolumePixelData::setData(vtkImageData *vtkImage)
{
    if (m_imageDataVTK)
    {
        m_imageDataVTK->ReleaseData();
    }
    m_imageDataVTK = vtkImage;
}

VolumePixelData::VoxelType* VolumePixelData::getScalarPointer(int x, int y, int z)
{
    return static_cast<VolumePixelData::VoxelType *>(this->getVtkData()->GetScalarPointer(x,y,z));
}

bool VolumePixelData::getVoxelValue(double coordinate[3], QVector<double> &voxelValue)
{
    int voxelIndex[3];
    double parametricCoordinates[3];
    int inside = this->getVtkData()->ComputeStructuredCoordinates(coordinate, voxelIndex, parametricCoordinates);

    if (inside == 1)
    {
        vtkIdType pointId = this->getVtkData()->ComputePointId(voxelIndex);
        int numberOfComponents = this->getVtkData()->GetNumberOfScalarComponents();
        voxelValue.resize(numberOfComponents);

        for (int i = 0; i < numberOfComponents; i++)
        {
            voxelValue[i] = this->getVtkData()->GetPointData()->GetScalars()->GetComponent(pointId, i);
        }

        return true;
    }
    else
    {
        return false;
    }
}

void VolumePixelData::convertToNeutralPixelData()
{
    // Creem un objecte vtkImageData "neutre"
    m_imageDataVTK = vtkSmartPointer<vtkImageData>::New();
    // Inicialitzem les dades
    m_imageDataVTK->SetOrigin(.0, .0, .0);
    m_imageDataVTK->SetSpacing(1., 1., 1.);
    m_imageDataVTK->SetDimensions(10, 10, 1);
    m_imageDataVTK->SetWholeExtent(0, 9, 0, 9, 0, 0);
    m_imageDataVTK->SetScalarTypeToShort();
    m_imageDataVTK->SetNumberOfScalarComponents(1);
    m_imageDataVTK->AllocateScalars();
    // Omplim el dataset perquè la imatge resultant quedi amb un cert degradat
    signed short *scalarPointer = (signed short *) m_imageDataVTK->GetScalarPointer();
    signed short value;
    for (int i = 0; i < 10; i++)
    {
        value = 150 - i * 20;
        if (i > 4)
        {
            value = 150 - (10 - i - 1)*20;
        }

        for (int j = 0; j < 10; j++)
        {
            *scalarPointer = value;
            *scalarPointer++;
        }
    }
}

} // End namespace udg
