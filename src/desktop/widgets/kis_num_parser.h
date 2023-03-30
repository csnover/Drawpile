/*
 *  SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_NUMPARSER_H
#define KIS_NUMPARSER_H

#include <QString>

/*!
 * \brief the namespace contains functions to transform math expression written as QString in numbers.
 *
 * Computation is done in a recursive way, maybe not the most efficient way compared to infix to postfix conversion before parsing.
 * (TODO: look if it need to be changed).
 */
namespace KisNumericParser {

    //! \brief parse an expression to a double.
    double parseSimpleMathExpr(QString const& expr, bool* noProblem = nullptr);

    //! \brief parse an expression to an int.
    int parseIntegerMathExpr(QString const& expr, bool* noProblem = nullptr);
}

#endif // KIS_NUMPARSER_H

