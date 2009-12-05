/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGSTOREIMAGES_H
#define UDGSTOREIMAGES_H

#include <QList>

struct T_ASC_Association;
struct T_ASC_Network;
struct T_DIMSE_C_StoreRSP;

class DcmDataset;

namespace udg {

class Status;
class PacsConnection;
class Image;

/**
	@author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class StoreImages{
public:
    StoreImages();

    /** This action sets the connection that we will use to connect to the pacs
     * @param   connection [in] Study's Open connection to the pacs
     */
   void setConnection( PacsConnection connection );

   /** Sets the network to use for retrieve the images
    * @param Network to use
    */
   void setNetwork ( T_ASC_Network * network );

   /** Guarda les imatges que s'especifiquen a la llista en el pacs establert per la connexió
    * @param ImageListStore de les imatges a enviar al PACS
    * @return indica estat del mètode
    */
   Status store( QList<Image*> imageListToStore );

private :
    T_ASC_Association *m_association; // request DICOM association;
    T_ASC_Network *m_network;
    //Indica números d'imatges enviades correctament/Imatges enviades però que ha retorna warning
    int m_numberOfStoredImagesSuccessful, m_numberOfStoredImagesWithWarning;

    ///Inicialitze els comptadors d'imatges per controlar quantes han fallat/s'han enviat....
    void initialitzeImagesCounters();

    ///Processa un resposta del Store SCP que no ha tingut l'Status Successfull
    void processResponseFromStoreSCP(T_DIMSE_C_StoreRSP *response, DcmDataset *statusDetail, QString filePathDicomObjectStoredFailed);

    ///Envia una image al PACS amb l'associació passada per paràmetre
    void storeSCU( T_ASC_Association * association , QString filePathToStore );

    ///Retorna un Status indicant com ha finalitzat l'operació C-Store
    Status getStatusStoreSCU(int numberOfImagesToStore);
};

}

#endif
