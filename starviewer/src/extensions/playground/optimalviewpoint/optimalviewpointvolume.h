/***************************************************************************
 *   Copyright (C) 2006 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#ifndef UDGMAGICMIRRORSVOLUME_H
#define UDGMAGICMIRRORSVOLUME_H

#include <QObject>

#include <vector>   // per std::vector<*>

#include "optimalviewpoint.h" // per OptimalViewpoint::TransferFunction

class vtkColorTransferFunction;
class vtkImageData;
class vtkPiecewiseFunction;
class vtkVolume;
class vtkVolumeProperty;
class vtkVolumeRayCastMapper;
class vtkVolumeRayCastCompositeFunction;
class vtkVolumeRayCastCompositeFunctionOptimalViewpoint;

namespace udg {

/**
 * Encapsula el tractament de vtkVolumes que corresponen a un mateix model:
 * assignar funcions de transferència i sincronitzar les transformacions.
 */
class OptimalViewpointVolume : public QObject {

    Q_OBJECT

public:

    OptimalViewpointVolume( vtkImageData * image );
    ~OptimalViewpointVolume();

    /// Retorna el vtkVolume corresponent a l'índex donat.
    vtkVolume * getMainVolume() const;
    vtkVolume * getPlaneVolume() const;

    void setShade( bool on );

    void setImageSampleDistance( double imageSampleDistance );

    void setSampleDistance( double sampleDistance );

    /**
     * Estableix la funció de transferència d'opacitat pel vtkVolume
     * corresponent a l'índex donat.
     */
    void setOpacityTransferFunction( vtkPiecewiseFunction * opacityTransferFunction);

    /**
     * Estableix la funció de transferència de color pel vtkVolume corresponent
     * a l'índex donat.
     */
    void setColorTransferFunction( vtkColorTransferFunction * colorTransferFunction);

    /**
     * Sincronitza les tranformacions de tots els vtkVolumes. Concretament,
     * aplica la transformació del vtkVolume amb índex 0 a tots els altres
     * vtkVolumes.
     */
    void synchronize();

    void handle( int rayId, int offset );
    void endRay( int rayId );

    unsigned char segmentateVolume( unsigned short iterations, unsigned char numberOfClusters, double noise );

    void setSegmentationFileName( QString name );




    // nous paràmetres
    void setOpacityForComputing( bool on );
    static const int INTERPOLATION_NEAREST_NEIGHBOUR = 0,
                     INTERPOLATION_LINEAR_INTERPOLATE_CLASSIFY = 1,
                     INTERPOLATION_LINEAR_CLASSIFY_INTERPOLATE = 2;
    void setInterpolation( int interpolation );
    void setSpecular( bool on );
    void setSpecularPower( double specularPower );




public slots:

    void setExcessEntropy( double excessEntropy );
    void setComputing( bool on = true );

private:

    /// Model de vòxels original.
    vtkImageData * m_image;
    vtkImageData * m_simplifiedImage;
    vtkImageData * m_segmentedImage;

    /// Vector de volums.
    vtkVolume * m_mainVolume;
    vtkVolume * m_planeVolume;

    /// Vector de mappers.
    vtkVolumeRayCastMapper * m_mainMapper;
    vtkVolumeRayCastMapper * m_planeMapper;

    /// Vector de funcions de ray-cast.
    vtkVolumeRayCastCompositeFunction * m_mainVolumeRayCastFunction;
    vtkVolumeRayCastCompositeFunctionOptimalViewpoint * m_planeVolumeRayCastFunction;

    /// Vector de funcions de transferència d'opacitat.
    vtkPiecewiseFunction * m_opacityTransferFunction;

    /// Vector de funcions de transferència de color.
    vtkColorTransferFunction * m_colorTransferFunction;

    /// Vector de propietats de volum.
    vtkVolumeProperty * m_volumeProperty;



    unsigned char * m_data;
    unsigned char * m_simplifiedData;
    unsigned char * m_segmentedData;
    int m_dataSize;

    double m_imageSampleDistance;
    double m_sampleDistance;

    double m_excessEntropy;

    QString m_segmentationFileName;


signals:

    void needsExcessEntropy();
    void visited( int rayId, unsigned char value );
    void rayEnd( int rayId );
    void adjustedTransferFunctionDefined( const OptimalViewpoint::TransferFunction & adjustedTransferFunction );


}; // end class OptimalViewpointVolume

}; // end namespace udg

#endif // UDGMAGICMIRRORSVOLUME_H
