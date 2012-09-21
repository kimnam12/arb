/*=============================================================================

    This file is part of ARB.

    ARB is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    ARB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ARB; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

=============================================================================*/
/******************************************************************************

    Copyright (C) 2011 William Hart
    Copyright (C) 2012 Fredrik Johansson

******************************************************************************/

#include "fmprb_poly.h"

void
_fmprb_poly_reverse(fmprb_struct * res, const fmprb_struct * poly, long len, long n)
{
    if (res == poly)
    {
        long i;

        for (i = 0; i < n / 2; i++)
        {
            fmprb_struct t = res[i];
            res[i] = res[n - 1 - i];
            res[n - 1 - i] = t;
        }

        for (i = 0; i < n - len; i++)
            fmprb_zero(res + i);
    }
    else
    {
        long i;

        for (i = 0; i < n - len; i++)
            fmprb_zero(res + i);

        for (i = 0; i < len; i++)
            fmprb_set(res + (n - len) + i, poly + (len - 1) - i);
    }
}

void
_fmprb_poly_div_series(fmprb_struct * Q,
    const fmprb_struct * A,
    const fmprb_struct * B, long n, long prec)
{
    fmprb_struct * Binv = _fmprb_vec_init(n);

    _fmprb_poly_inv_series(Binv, B, n, prec);
    _fmprb_poly_mullow(Q, Binv, n, A, n, n, prec);

    _fmprb_vec_clear(Binv, n);
}

void
_fmprb_poly_div(fmprb_struct * Q,
    const fmprb_struct * A, long lenA,
    const fmprb_struct * B, long lenB, long prec)
{
    const long lenQ = lenA - lenB + 1;
    fmprb_struct * Arev, * Brev;

    Arev = _fmprb_vec_init(2 * lenQ);
    Brev = Arev + lenQ;

    _fmprb_poly_reverse(Arev, A + (lenA - lenQ), lenQ, lenQ);

    if (lenB >= lenQ)
    {
        _fmprb_poly_reverse(Brev, B + (lenB - lenQ), lenQ, lenQ);
    }
    else
    {
        _fmprb_poly_reverse(Brev, B, lenB, lenB);
        _fmprb_vec_zero(Brev + lenB, lenQ - lenB);
    }

    _fmprb_poly_div_series(Q, Arev, Brev, lenQ, prec);
    _fmprb_poly_reverse(Q, Q, lenQ, lenQ);

    _fmprb_vec_clear(Arev, 2 * lenQ);
}

void _fmprb_poly_divrem(fmprb_struct * Q, fmprb_struct * R,
    const fmprb_struct * A, long lenA,
    const fmprb_struct * B, long lenB, long prec)
{
    const long lenQ = lenA - lenB + 1;
    _fmprb_poly_div(Q, A, lenA, B, lenB, prec);

    if (lenB > 1)
    {
        if (lenQ >= lenB - 1)
            _fmprb_poly_mullow(R, Q, lenQ, B, lenB - 1, lenB - 1, prec);
        else
            _fmprb_poly_mullow(R, B, lenB - 1, Q, lenQ, lenB - 1, prec);
        _fmprb_vec_sub(R, A, R, lenB - 1, prec);
    }
}

void _fmprb_poly_rem(fmprb_struct * R,
    const fmprb_struct * A, long lenA,
    const fmprb_struct * B, long lenB, long prec)
{
    const long lenQ = lenA - lenB + 1;
    fmprb_struct * Q = _fmprb_vec_init(lenQ);
    _fmprb_poly_divrem(Q, R, A, lenA, B, lenB, prec);
    _fmprb_vec_clear(Q, lenQ);
}

void fmprb_poly_divrem(fmprb_poly_t Q, fmprb_poly_t R,
                             const fmprb_poly_t A, const fmprb_poly_t B, long prec)
{
    const long lenA = A->length, lenB = B->length;

    if (lenB == 0 || fmprb_contains_zero(B->coeffs + lenB - 1))
    {
        printf("Exception: division by zero in fmprb_poly_divrem\n");
        abort();
    }

    if (lenA < lenB)
    {
        fmprb_poly_set(R, A);
        fmprb_poly_zero(Q);
        return;
    }

    if (Q == A || Q == B)
    {
        fmprb_poly_t T;
        fmprb_poly_init(T);
        fmprb_poly_divrem(T, R, A, B, prec);
        fmprb_poly_swap(Q, T);
        fmprb_poly_clear(T);
        return;
    }

    if (R == A || R == B)
    {
        fmprb_poly_t U;
        fmprb_poly_init(U);
        fmprb_poly_divrem(Q, U, A, B, prec);
        fmprb_poly_swap(R, U);
        fmprb_poly_clear(U);
        return;
    }

    fmprb_poly_fit_length(Q, lenA - lenB + 1);
    fmprb_poly_fit_length(R, lenB - 1);

    _fmprb_poly_divrem(Q->coeffs, R->coeffs, A->coeffs, lenA,
                                   B->coeffs, lenB, prec);

    _fmprb_poly_set_length(Q, lenA - lenB + 1);
    _fmprb_poly_set_length(R, lenB - 1);
    _fmprb_poly_normalise(R);
}
