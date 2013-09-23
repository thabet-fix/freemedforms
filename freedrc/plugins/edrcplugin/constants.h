/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2013 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *  Main developer: Eric MAEKER, <eric.maeker@gmail.com>                   *
 *  Contributors:                                                          *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#ifndef EDRCPLUGIN_CONSTANTS_H
#define EDRCPLUGIN_CONSTANTS_H

/**
 * \file constants.h
 * \author Eric Maeker
 * \version 0.10.0
 * \date 18 June 2013
*/

namespace eDRC {
namespace Constants {

// Some versions
const char * const VALIDATOR_VERSION = "2013.06";

// Xml tags
const char * const XML_EXTRA_TAG = "CrExtra";

// Settings params
const char * const S_CR_USE_MODERNLABEL = "edrc/crlabel/modern";
const char * const S_CR_MANDATORYLABEL_IN_BOLD = "edrc/crlabel/mandatoryinbold";
const char * const S_REALTIME_CR_CODING_CHECKING = "edrc/coding/realTimeChecking";
const char * const S_CR_EDITOR_MANAGES_USERCOMMENTS = "edrc/editor/managesUserComments";
const char * const S_TOKEN_HTMLGLOBALMASK = "edrc/htmlmask/global";
const char * const S_TOKEN_HTMLCRITERIASMASK = "edrc/htmlmask/crit";

// Tokens old style
const char * const TOKEN_CR_DATABASE_VERSION = "CR_DATABASE_VERSION";
const char * const TOKEN_CR_CODING_VALIDITY = "CR_CODING_VALIDITY";
const char * const TOKEN_CR_CODING_VALIDATOR_VERSION = "CR_CODING_VALIDATOR_VERSION";
const char * const TOKEN_CR_ID = "CR_ID";
const char * const TOKEN_CR_LABEL = "CR_LABEL";
const char * const TOKEN_CR_CRITERIAS = "CR_CRITERIAS";
const char * const TOKEN_CR_DIAGNOSTIC_POSITION_CODE = "CR_DIAGNOSTIC_POSITION_CODE";
const char * const TOKEN_CR_CR_DIAGNOSTIC_POSITION_FULL_LABEL = "CR_DIAGNOSTIC_POSITION_FULL_LABEL";
const char * const TOKEN_CR_FOLLOWUP_CODE = "CR_FOLLOWUP_CODE";
const char * const TOKEN_CR_FOLLOWUP_FULL_LABEL = "CR_FOLLOWUP_FULL_LABEL";
const char * const TOKEN_CR_CHRONIC_DISEASE = "CR_CHRONIC_DISEASE";
const char * const TOKEN_CR_CR_SYMPTOMATIC_STATE = "CR_SYMPTOMATIC_STATE";
const char * const TOKEN_CR_CR_ICD10_CODES = "CR_ICD10_CODES";
const char * const TOKEN_CR_ICD10_LABELS = "CR_ICD10_LABELS";
const char * const TOKEN_CR_ICD10_CODES_AND_LABELS = "CR_ICD10_CODES_AND_LABELS";

const char * const TOKEN_CR_GLOBAL_HTML_COMMENT = "CR_GLOBAL_HTML_COMMENT";
const char * const TOKEN_CR_CRITERIAS_HTML_COMMENT = "CR_CRITERIAS_HTML_COMMENT";

const char * const TOKEN_CRITERIA_ID = "CRITERIA_ID";
const char * const TOKEN_CRITERIA_DEFAULT_LABEL = "CRITERIA_LABEL";
const char * const TOKEN_CRITERIA_MODERN_LABEL = "CRITERIA_MODERN_LABEL";
// TODO: add CR_DATABASE_VERSION, CR_CODING_VALIDITY

} // namespace Constants
} // namespace eDRC

#endif // EDRCPLUGIN_CONSTANTS_H
