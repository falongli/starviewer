/***************************************************************************
 *   Copyright (C) 2007 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.edu/GGG/index.html                                    *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/


#include "qoptimalviewpointextension.h"

#include <QMessageBox>

#include "optimalviewpoint.h"
#include "optimalviewpointparameters.h"

#include "logging.h"
#include "volume.h"

#include <iostream>
#include <QSettings>
#include <QFileDialog>


namespace udg {


QOptimalViewpointExtension::QOptimalViewpointExtension( QWidget * parent )
    : QWidget( parent )
{
    setupUi( this );

    m_parameters = new OptimalViewpointParameters( this );

    m_inputParametersWidget->setParameters( m_parameters );

    m_method = new OptimalViewpoint( this );
    m_method->setParameters( m_parameters );
    m_method->setMainRenderer( m_viewerWidget->getRenderer() );

    m_parameters->init();

    connect( m_openSegmentationFilePushButton, SIGNAL( clicked() ), SLOT( openSegmentationFile() ) );
    connect( m_segmentationOkPushButton, SIGNAL( clicked() ), SLOT( doSegmentation() ) );




    connect( m_inputParametersWidget, SIGNAL( executionRequested() ), SLOT( execute() ) );

    connect( m_inputParametersWidget, SIGNAL( newMethod2Requested(int,bool) ), m_method, SLOT( newMethod2(int,bool) ) );

    connect( m_method, SIGNAL( scalarRange(unsigned char,unsigned char) ), SLOT( setScalarRange(unsigned char,unsigned char) ) );


    m_automaticSegmentationWidget->hide();
    m_regularSegmentationWidget->hide();

    m_segmentationFileChosen = false;


    connect( m_parameters, SIGNAL( changed(int) ), SLOT( readParameter(int) ) );

    connect( m_visualizationOkPushButton, SIGNAL( clicked() ), SLOT( doVisualization() ) );

    createConnections();

    connect( m_obscurancesPushButton, SIGNAL( clicked() ), this, SLOT( computeObscurances() ) );
    connect( m_saliencyPushButton, SIGNAL( clicked() ), this, SLOT( computeSaliency() ) );

    connect( m_comboNumberOfPlanes, SIGNAL( currentIndexChanged(const QString &) ), SLOT( setNumberOfPlanes(const QString &) ) );
    connect( m_viewpointSelectionOkPushButton, SIGNAL( clicked() ), SLOT( doViewpointSelection() ) );
    connect( m_updatePlaneRenderPushButton, SIGNAL( clicked() ), SLOT( renderPlane() ) );

    m_parameters->setUpdatePlane( -1 ); // de moment ho poso aquí, perquè s'ha d'inicialitzar a algun lloc

    connect( m_viewpointEntropiesOkPushButton, SIGNAL( clicked() ), SLOT( computeViewpointEntropies() ) );
}


QOptimalViewpointExtension::~QOptimalViewpointExtension()
{
//     delete m_method;
}


void QOptimalViewpointExtension::setInput( Volume * input )
{
    Q_ASSERT( input );

    Volume * volume = input;

    if ( volume->getNumberOfPhases() > 1 ) volume = volume->getPhaseVolume( 14 );

    m_method->setImage( volume->getVtkData() );
    int dims[3];
    DEBUG_LOG( "input->getDimensions" );
    volume->getDimensions( dims );
    DEBUG_LOG( "m_inputParametersWidget->setNumberOfSlices" );
    m_inputParametersWidget->setNumberOfSlices( dims[2] );
    DEBUG_LOG( "end setInput" );
}


void QOptimalViewpointExtension::doSegmentation()
{
    if ( m_automaticSegmentationRadioButton->isChecked() )
    {
        m_method->doAutomaticSegmentation(
                                           m_segmentationIterationsSpinBox->value(),
                                           m_segmentationBlockLengthSpinBox->value(),
                                           m_segmentationNumberOfClustersSpinBox->value(),
                                           m_segmentationNoiseDoubleSpinBox->value(),
                                           m_segmentationImageSampleDistanceDoubleSpinBox->value(),
                                           m_segmentationSampleDistanceDoubleSpinBox->value()
                                         );
    }
    else if ( m_loadSegmentationRadioButton->isChecked() )
    {
        if ( !m_segmentationFileChosen )
        {
            QMessageBox::warning( this, tr("No segmentation file chosen"),
                                  tr("Please, choose a segmentation file or do another type of segmentation.") );
            return;
        }

        if ( !m_method->doLoadSegmentation( m_segmentationFileLabel->text() ) )
        {
            QMessageBox::critical( this, tr("Segmentation error"),
                                   QString( tr("Cannot load segmentation from file %1.") ).arg( m_segmentationFileLabel->text() ) );
            return;
        }
    }
    else //if ( m_regularSegmentationRadioButton->isChecked() )
    {
        m_method->doRegularSegmentation( m_segmentationNumberOfBinsSpinBox->value() );
    }

    m_numberOfClustersLabel->setText( QString("<b>%1 clusters</b>").arg( static_cast<short>( m_method->getNumberOfClusters() ) ) );
    DEBUG_LOG( QString("nclusters = %1").arg( static_cast<short>( m_method->getNumberOfClusters() ) ) );

    m_loadSegmentationRadioButton->setDisabled( true );
    m_loadSegmentationWidget->setDisabled( true );
    m_automaticSegmentationRadioButton->setDisabled( true );
    m_automaticSegmentationWidget->setDisabled( true );
    m_regularSegmentationRadioButton->setDisabled( true );
    m_regularSegmentationWidget->setDisabled( true );
//     m_segmentationOkPushButton->setDisabled( true );
    toggleSegmentationParameters();

    disconnect( m_segmentationOkPushButton, SIGNAL( clicked() ), this, SLOT( doSegmentation() ) );
    connect( m_segmentationOkPushButton, SIGNAL( clicked() ), SLOT( toggleSegmentationParameters() ) );

    // De moment cal posar aquestes dues línies. Potser es pot arreglar perquè no calguin.
    m_method->setNumberOfPlanes( 0 );
    m_method->setTransferFunction( m_parameters->getTransferFunctionObject() );
    m_viewerWidget->render();

    m_segmentationWidget->setChecked( false );
    m_visualizationOkPushButton->setEnabled( true );
    m_visualizationWidget->setChecked( true );
    m_viewpointSelectionOkPushButton->setEnabled( true );
    m_viewpointEntropiesOkPushButton->setEnabled( true );
}


void QOptimalViewpointExtension::openSegmentationFile()
{
    QSettings settings;

    settings.beginGroup( "OptimalViewpoint" );

    QString segmentationFileDir = settings.value( "segmentationFiledir", QString() ).toString();

    QString segmentationFileName =
            QFileDialog::getOpenFileName( this, tr("Open segmentation file"),
                                          segmentationFileDir, tr("Segmentation files (*.seg);;All files (*)") );

    if ( !segmentationFileName.isNull() )
    {
        m_segmentationFileLabel->setText( segmentationFileName );
        m_segmentationFileChosen = true;

        QFileInfo segmentationFileInfo( segmentationFileName );
        settings.setValue( "segmentationFileDir", segmentationFileInfo.absolutePath() );
    }

    settings.endGroup();
}


void QOptimalViewpointExtension::execute()
{
    writeAllParameters();
    // nous paràmetres

    m_method->setOpacityForComputing( m_parameters->getComputeWithOpacity() );
    m_method->setUpdatePlane( m_parameters->getUpdatePlane() );
    m_method->setSimilarityThreshold( m_parameters->getSimilarityThreshold() );

    bool renderCluster = m_parameters->getCluster();
    if ( renderCluster ) m_method->setClusterLimits( m_parameters->getClusterFirst(), m_parameters->getClusterLast() );
    m_method->setRenderCluster( renderCluster );

    m_method->setReadExtentFromFile( m_parameters->getReadExtentFromFile() );



    m_method->updatePlanes();
    std::cout << "OVD: update planes" << std::endl;
//     m_viewer->render();
    m_viewerWidget->render();

    if ( m_method->resultsChanged() )
    {
        std::vector<double> * entropyRateResults = m_method->getEntropyRateResults();
        std::vector<double> * excessEntropyResults = m_method->getExcessEntropyResults();

        QMessageBox * resultsDialog = new QMessageBox( tr("Results"), "", QMessageBox::Information,
                QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, m_viewerWidget );
        resultsDialog->setModal( false );
        resultsDialog->setAttribute( Qt::WA_DeleteOnClose );

        QString text = "<table cellspacing=\"8\"><tr><td></td><td align=\"center\"><b><i><u>entropy rate</u></i></b></td><td align=\"center\"><b><i><u>excess entropy</u></i></b></td></tr>";
        QString planeString = tr("Plane");
        for ( unsigned char i = 1; i <= m_parameters->getNumberOfPlanes(); i++ )
            text += "<tr><td><b>" + planeString + QString( " %1:</b></td><td align=\"center\">%2</td><td align=\"center\">%3</td></tr>" ).arg( i ).arg( (*entropyRateResults)[i], 0, 'g', 7 ).arg( (*excessEntropyResults)[i], 0, 'g', 7 );
        text += "</table>";

        resultsDialog->setText( text );
        resultsDialog->show();

        delete entropyRateResults;
        delete excessEntropyResults;
    }
}


void QOptimalViewpointExtension::setScalarRange( unsigned char rangeMin, unsigned char rangeMax )
{
    m_inputParametersWidget->setRangeMax( rangeMax );
    unsigned short maximum = rangeMax - rangeMin + 1;
    if ( maximum < m_segmentationNumberOfBinsSpinBox->maximum() )
        m_segmentationNumberOfBinsSpinBox->setMaximum( maximum );
}


void QOptimalViewpointExtension::renderPlane()
{
    m_method->renderPlanes( m_updatePlaneSpinBox->value() );
}


void QOptimalViewpointExtension::toggleSegmentationParameters()
{
    if ( m_segmentationLine->isVisible() )
    {
        m_segmentationOkPushButton->setText( tr("Show parameters") );
        m_loadSegmentationRadioButton->hide();
        m_loadSegmentationWidget->hide();
        m_automaticSegmentationRadioButton->hide();
        m_automaticSegmentationWidget->hide();
        m_regularSegmentationRadioButton->hide();
        m_regularSegmentationWidget->hide();
        m_segmentationLine->hide();
    }
    else
    {
        m_segmentationOkPushButton->setText( tr("Hide parameters") );
        m_loadSegmentationRadioButton->show();
        if ( m_loadSegmentationRadioButton->isChecked() ) m_loadSegmentationWidget->show();
        m_automaticSegmentationRadioButton->show();
        if ( m_automaticSegmentationRadioButton->isChecked() ) m_automaticSegmentationWidget->show();
        m_regularSegmentationRadioButton->show();
        if ( m_regularSegmentationRadioButton->isChecked() ) m_regularSegmentationWidget->show();
        m_segmentationLine->show();
    }
}


void QOptimalViewpointExtension::readParameter( int index )
{
    if( !m_parameters )
    {
        DEBUG_LOG("OptimalViewpointInputParametersForm: No hi ha paràmetres establerts");
    }
    else
    {
        switch ( index )
        {
            case OptimalViewpointParameters::Shade:
                m_shadeCheckBox->setChecked( m_parameters->getShade() );
                break;

            case OptimalViewpointParameters::Interpolation:
                m_interpolationComboBox->setCurrentIndex( m_parameters->getInterpolation() );
                break;

            case OptimalViewpointParameters::Specular:
                m_specularCheckBox->setChecked( m_parameters->getSpecular() );
                break;

            case OptimalViewpointParameters::SpecularPower:
                m_specularPowerDoubleSpinBox->setValue( m_parameters->getSpecularPower() );
                break;

            case OptimalViewpointParameters::Obscurances:
                m_obscurancesCheckBox->setChecked( m_parameters->getObscurances() );
                break;

            case OptimalViewpointParameters::ObscurancesFactor:
                m_obscurancesFactorDoubleSpinBox->setValue( m_parameters->getObscurancesFactor() );
                break;

            case OptimalViewpointParameters::NumberOfPlanes:
                m_comboNumberOfPlanes->setCurrentIndex( m_comboNumberOfPlanes->findText( QString::number( m_parameters->getNumberOfPlanes() ) ) );
                break;

            case OptimalViewpointParameters::UpdatePlane:
                m_updatePlaneSpinBox->setValue( m_parameters->getUpdatePlane() );
                break;

            case OptimalViewpointParameters::VisualizationImageSampleDistance:
                m_doubleSpinBoxVisualizationImageSampleDistance->setValue( m_parameters->getVisualizationImageSampleDistance() );
                break;

            case OptimalViewpointParameters::VisualizationSampleDistance:
                m_doubleSpinBoxVisualizationSampleDistance->setValue( m_parameters->getVisualizationSampleDistance() );
                break;

            case OptimalViewpointParameters::VisualizationBlockLength:
                m_spinBoxVisualizationBlockLength->setValue( m_parameters->getVisualizationBlockLength() );
                break;
        }
    }
}

void QOptimalViewpointExtension::writeAllParameters()
{
    if( !m_parameters )
    {
        DEBUG_LOG("OptimalViewpointInputParametersForm: No hi ha paràmetres establerts");
    }
    else
    {
    }
}


void QOptimalViewpointExtension::doVisualization()
{
    m_parameters->setShade( m_shadeCheckBox->isChecked() );
    if ( m_interpolationComboBox->currentIndex() < 0 )
        m_interpolationComboBox->setCurrentIndex( 0 );
    m_parameters->setInterpolation( m_interpolationComboBox->currentIndex() );
    m_parameters->setSpecular( m_specularCheckBox->isChecked() );
    m_parameters->setSpecularPower( m_specularPowerDoubleSpinBox->value() );
    m_parameters->setObscurances( m_obscurancesCheckBox->isChecked() );
    m_parameters->setObscurancesFactor( m_obscurancesFactorDoubleSpinBox->value() );

    m_viewerWidget->render();
}


void QOptimalViewpointExtension::createConnections()
{
//     connect( m_interpolationComboBox, SIGNAL( currentIndexChanged(int) ), m_parameters, SLOT( setInterpolation(int) ) );
//     connect( m_shadeCheckBox, SIGNAL( toggled(bool) ), m_parameters, SLOT( setShade(bool) ) );
}


void QOptimalViewpointExtension::computeObscurances()
{
    m_method->computeObscurances( m_obscuranceDirectionsSpinBox->value(),
                                  m_obscuranceMaximumDistanceDoubleSpinBox->value(),
                                  m_obscuranceFunctionComboBox->currentIndex(),
                                  m_obscuranceVariantComboBox->currentIndex() );
}


void QOptimalViewpointExtension::doViewpointSelection()
{
    m_parameters->setNumberOfPlanes( m_comboNumberOfPlanes->currentText().toUShort() );
    m_parameters->setUpdatePlane( m_updatePlaneSpinBox->value() );
}


void QOptimalViewpointExtension::setNumberOfPlanes( const QString & numberOfPlanes )
{
    m_updatePlaneSpinBox->setMaximum( numberOfPlanes.toInt() );
}


void QOptimalViewpointExtension::computeSaliency()
{
    m_method->computeSaliency();
}


void QOptimalViewpointExtension::computeViewpointEntropies()
{
    m_parameters->setVisualizationImageSampleDistance( m_doubleSpinBoxVisualizationImageSampleDistance->value() );
    m_parameters->setVisualizationSampleDistance( m_doubleSpinBoxVisualizationSampleDistance->value() );
    m_parameters->setVisualizationBlockLength( m_spinBoxVisualizationBlockLength->value() );
    m_method->computeViewpointEntropies();

    {
        std::vector<double> * entropyRateResults = m_method->getEntropyRateResults();
        std::vector<double> * excessEntropyResults = m_method->getExcessEntropyResults();

        QMessageBox * resultsDialog = new QMessageBox( tr("Results"), "", QMessageBox::Information,
                QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, m_viewerWidget );
        resultsDialog->setModal( false );
        resultsDialog->setAttribute( Qt::WA_DeleteOnClose );

        QString text = "<table cellspacing=\"8\"><tr><td></td><td align=\"center\"><b><i><u>entropy rate</u></i></b></td><td align=\"center\"><b><i><u>excess entropy</u></i></b></td></tr>";
        QString planeString = tr("Plane");
        for ( unsigned char i = 1; i <= m_parameters->getNumberOfPlanes(); i++ )
            text += "<tr><td><b>" + planeString + QString( " %1:</b></td><td align=\"center\">%2</td><td align=\"center\">%3</td></tr>" ).arg( i ).arg( (*entropyRateResults)[i], 0, 'g', 7 ).arg( (*excessEntropyResults)[i], 0, 'g', 7 );
        text += "</table>";

        resultsDialog->setText( text );
        resultsDialog->show();

        delete entropyRateResults;
        delete excessEntropyResults;
    }
}


}
