/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/



#ifndef UDGLOCALDATABASEBASEDAL_H
#define UDGLOCALDATABASEBASEDAL_H

class QChar;
class QSqlError;
class QString;
class QVariant;

namespace udg {

class DatabaseConnection;

/**
    Classe base de les que hereden totes les classes que implementen una DAL per accés a dades
  */
class LocalDatabaseBaseDAL {
public:
    LocalDatabaseBaseDAL(DatabaseConnection *dbConnection);

    /// Retorna l'últim error produït
    QSqlError getLastError();

    /// Formata l'string de forma que no contingui caràcters extranys que puguin fer
    /// que l'execució d'una comanda SQL sigui incorrecta
    static QString formatTextToValidSQLSyntax(QString string);

    /// Formata un qchar perquè no contingui caràcters estranys o Nulls que pugin fer que la sentència sql sigui incorrecte
    static QString formatTextToValidSQLSyntax(QChar qchar);

protected:

    /// Converts the given text to a QString, interpreting the input as either UTF-8 or Latin-1 depending on its content.
    static QString convertToQString(const QVariant &text);

    /// Ens fa un ErrorLog d'una sentència sql. No es té en compte l'error és SQL_CONSTRAINT (clau duplicada)
    void logError(const QString &sqlSentence);

protected:
    DatabaseConnection *m_dbConnection;

};
}

#endif
