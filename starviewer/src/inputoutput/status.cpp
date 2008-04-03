/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#include "status.h"

#include <ofcond.h> //provide the OFcondition structure and his members

namespace udg{

Status::Status()
{
    m_numberError=0;
    m_success = true;
    m_descText = "";
}

bool Status::good() const
{
	return m_success;
}

QString Status::text() const
{
    return m_descText;
}

int Status::code() const
{
	return m_numberError;
}

Status Status::setStatus( const OFCondition status )
{
	m_descText = status.text();
	m_success = status.good();
	m_numberError = status.code();

    return *this;
}

Status Status::setStatus( QString desc , bool ok , int numError )
{
    m_descText = desc;
    m_success = ok;
    m_numberError = numError;

    return *this;
}

};
