/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGPRESENTATIONSTATEFILLERSTEP_H
#define UDGPRESENTATIONSTATEFILLERSTEP_H

#include "patientfillerstep.h"

namespace udg {

class Patient;
class Series;

/**
Mòdul que s'encarrega d'omplir la informació d'objectes PresentationState. Un dels seus prerequisits serà que s'hagi superat el mòdul DICOMFileClassifierFillerStep. Les Series a processar han de de ser de modalitat PR.

	@author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class PresentationStateFillerStep : public PatientFillerStep
{
public:
    PresentationStateFillerStep();

    ~PresentationStateFillerStep();

    bool fill();

private:
    /// Mètodes per processar la informació específica de pacient,series i imatge
    void processPatient( Patient *patient );
    void processSeries( Series *series );
    // TODO mètode per implementar
    void processPresentationState();

};

}

#endif
