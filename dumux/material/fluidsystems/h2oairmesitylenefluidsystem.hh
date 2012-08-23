// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   Copyright (C) 2011-2012 by Holger Class                                 *
 *   Copyright (C) 2009-2012 by Andreas Lauser                               *
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 *****************************************************************************/
/*!
 * \file
 * \ingroup Fluidsystems
 *
 * \brief A fluid system with water, gas and NAPL as phases and
 *        \f$H_2O\f$ and \f$Air\f$ and \f$NAPL (contaminant)\f$ as components.
 */
#ifndef DUMUX_H2O_AIR_MESITYLENE_FLUID_SYSTEM_HH
#define DUMUX_H2O_AIR_MESITYLENE_FLUID_SYSTEM_HH

#include "basefluidsystem.hh"
#include "nullparametercache.hh"

#include <dumux/material/idealgas.hh>
#include <dumux/material/components/n2.hh>
#include <dumux/material/components/air.hh>
#include <dumux/material/components/h2o.hh>
#include <dumux/material/components/simpleh2o.hh>
#include <dumux/material/components/mesitylene.hh>
#include <dumux/material/components/tabulatedcomponent.hh>
#include <dumux/material/binarycoefficients/h2o_air.hh>
#include <dumux/material/binarycoefficients/h2o_n2.hh>
#include <dumux/material/binarycoefficients/h2o_mesitylene.hh>
#include <dumux/material/binarycoefficients/air_mesitylene.hh>

#include <iostream>

namespace Dumux
{
namespace FluidSystems
{

/*!
 * \ingroup Fluidsystems
 * \brief A compositional fluid with water and molecular nitrogen as
 *        components in both, the liquid and the gas phase.
 */
template <class Scalar>
class H2OAirMesitylene
    : public BaseFluidSystem<Scalar, H2OAirMesitylene<Scalar> >
{
    typedef H2OAirMesitylene<Scalar> ThisType;
    typedef BaseFluidSystem<Scalar, ThisType> Base;

    typedef Dumux::H2O<Scalar> IapwsH2O;
    typedef Dumux::TabulatedComponent<Scalar, IapwsH2O, /*alongVaporPressure=*/false> TabulatedH2O;

public:
    typedef NullParameterCache ParameterCache;

    typedef Dumux::Mesitylene<Scalar> NAPL;
    typedef Dumux::Air<Scalar> Air;
    //typedef SimpleH2O H2O;
    typedef TabulatedH2O H2O;
    //typedef IapwsH2O H2O;
    
    static const int numPhases = 3;
    static const int numComponents = 3;

    static const int wPhaseIdx = 0; // index of the water phase
    static const int nPhaseIdx = 1; // index of the NAPL phase
    static const int gPhaseIdx = 2; // index of the gas phase

    static const int H2OIdx = 0;
    static const int NAPLIdx = 1;
    static const int airIdx = 2;

    static const int wCompIdx = H2OIdx;
    static const int cCompIdx = NAPLIdx;
    static const int aCompIdx = airIdx;

    /*!
     * \brief Initialize the fluid system's static parameters generically
     *
     * If a tabulated H2O component is used, we do our best to create
     * tables that always work.
     */
    static void init()
    {
        init(/*tempMin=*/273.15,
             /*tempMax=*/623.15,
             /*numTemp=*/100,
             /*pMin=*/0.0,
             /*pMax=*/20e6,
             /*numP=*/200);
    }

    /*!
     * \brief Initialize the fluid system's static parameters using
     *        problem specific temperature and pressure ranges
     *
     * \param tempMin The minimum temperature used for tabulation of water [K]
     * \param tempMax The maximum temperature used for tabulation of water [K]
     * \param nTemp The number of ticks on the temperature axis of the  table of water
     * \param pressMin The minimum pressure used for tabulation of water [Pa]
     * \param pressMax The maximum pressure used for tabulation of water [Pa]
     * \param nPress The number of ticks on the pressure axis of the  table of water
     */
    static void init(Scalar tempMin, Scalar tempMax, unsigned nTemp,
                     Scalar pressMin, Scalar pressMax, unsigned nPress)
    {
        if (H2O::isTabulated) {
            std::cout << "Initializing tables for the H2O fluid properties ("
                      << nTemp*nPress
                      << " entries).\n";

            TabulatedH2O::init(tempMin, tempMax, nTemp,
                               pressMin, pressMax, nPress);
        }
    }


    /*!
     * \brief Return whether a phase is liquid
     *
     * \param phaseIdx The index of the fluid phase to consider
     */
    static constexpr bool isLiquid(int phaseIdx)
    { 
        //assert(0 <= phaseIdx && phaseIdx < numPhases);
        return phaseIdx != gPhaseIdx; 
    }

    /*!
     * \brief Returns true iff a fluid is considered an ideal gas
     */
    static constexpr bool isIdealGas(int phaseIdx)
    { return phaseIdx == gPhaseIdx && H2O::gasIsIdeal() && Air::gasIsIdeal() && NAPL::gasIsIdeal(); }

    /*!
     * \brief Returns true if and only if a fluid phase is assumed to
     *        be an ideal mixture.
     *
     * We define an ideal mixture as a fluid phase where the fugacity
     * coefficients of all components times the pressure of the phase
     * are indepent on the fluid composition. This assumtion is true
     * if Henry's law and Rault's law apply. If you are unsure what
     * this function should return, it is safe to return false. The
     * only damage done will be (slightly) increased computation times
     * in some cases.
     *
     * \param phaseIdx The index of the fluid phase to consider
     */
    static constexpr bool isIdealMixture(int phaseIdx)
    {
        //assert(0 <= phaseIdx && phaseIdx < numPhases);
        // we assume Henry's and Rault's laws for the water phase and
        // and no interaction between gas molecules of different
        // components, so all phases are ideal mixtures!
        return true;
    }

    /*!
     * \brief Returns true if and only if a fluid phase is assumed to
     *        be compressible.
     *
     * Compressible means that the partial derivative of the density
     * to the fluid pressure is always larger than zero.
     *
     * \param phaseIdx The index of the fluid phase to consider
     */
    static constexpr bool isCompressible(int phaseIdx)
    {
        //assert(0 <= phaseIdx && phaseIdx < numPhases);
        // gases are always compressible
        return (phaseIdx == gPhaseIdx) 
            ? true
            : (phaseIdx == wPhaseIdx)
            ? H2O::liquidIsCompressible()
            : NAPL::liquidIsCompressible();
    }

    /*!
     * \brief Return the human readable name of a phase (used in indices)
     */
    static const char *phaseName(int phaseIdx)
    {
        switch (phaseIdx) {
        case wPhaseIdx: return "w";
        case nPhaseIdx: return "n";
        case gPhaseIdx: return "g";;
        };
        DUNE_THROW(Dune::InvalidStateException, "Invalid phase index " << phaseIdx);
    }

    /*!
     * \brief Return the human readable name of a component (used in indices)
     */
    static const char *componentName(int compIdx)
    {
        switch (compIdx) {
        case H2OIdx: return H2O::name();
        case airIdx: return Air::name();
        case NAPLIdx: return NAPL::name();
        };
        DUNE_THROW(Dune::InvalidStateException, "Invalid component index " << compIdx);
    }

    /*!
     * \brief Return the molar mass of a component in [kg/mol].
     */
    static constexpr Scalar molarMass(int compIdx)
    {
        return
            (compIdx == H2OIdx) 
            ? H2O::molarMass()
            : (compIdx == airIdx)
            ? Air::molarMass()
            : (compIdx == NAPLIdx)
            ? NAPL::molarMass()
            : -1e10;
        //DUNE_THROW(Dune::InvalidStateException, "Invalid component index " << compIdx);
    }

    /*!
     * \brief Given all mole fractions in a phase, return the phase
     *        density [kg/m^3].
     */
    template <class FluidState>
    static Scalar density(const FluidState &fluidState,
                          const ParameterCache &paramCache,
                          int phaseIdx)
    {
        Scalar T = fluidState.temperature(phaseIdx) ;
        Scalar p = fluidState.pressure(phaseIdx);

        if (phaseIdx == wPhaseIdx) {
            // See: Ochs 2008
            // \todo: proper citation
            Scalar rholH2O = H2O::liquidDensity(T, p);
            Scalar clH2O = rholH2O/H2O::molarMass();

            // this assumes each dissolved molecule displaces exactly one
            // water molecule in the liquid
            return
                clH2O*(H2O::molarMass()*fluidState.moleFraction(wPhaseIdx, H2OIdx)
                       +
                       Air::molarMass()*fluidState.moleFraction(wPhaseIdx, airIdx)
                       +
                       NAPL::molarMass()*fluidState.moleFraction(wPhaseIdx, NAPLIdx));
        }
        else if (phaseIdx == nPhaseIdx) {
            // assume pure NAPL for the NAPL phase
            Scalar p = NAPL::liquidIsCompressible()?fluidState.pressure(phaseIdx):1e100;
            return NAPL::liquidDensity(T, p);
        }

        assert (phaseIdx == gPhaseIdx);
        Scalar pH2O =
            fluidState.moleFraction(gPhaseIdx, H2OIdx)  *
            fluidState.pressure(gPhaseIdx);
        Scalar pAir =
            fluidState.moleFraction(gPhaseIdx, airIdx)  *
            fluidState.pressure(gPhaseIdx);
        Scalar pNAPL =
            fluidState.moleFraction(gPhaseIdx, NAPLIdx)  *
            fluidState.pressure(gPhaseIdx);
        return
            H2O::gasDensity(T, pH2O) +
            Air::gasDensity(T, pAir) +
            NAPL::gasDensity(T, pNAPL);
    }

    /*!
     * \brief Return the viscosity of a phase.
     */
    template <class FluidState>
    static Scalar viscosity(const FluidState &fluidState,
                            const ParameterCache &paramCache,
                            int phaseIdx)
    {
        Scalar T = fluidState.temperature(phaseIdx);
        Scalar p = fluidState.pressure(phaseIdx);

        if (phaseIdx == wPhaseIdx) {
            // assume pure water viscosity

            return H2O::liquidViscosity(T,
                                        p);
        }
        else if (phaseIdx == nPhaseIdx) {
            // assume pure NAPL viscosity
            return NAPL::liquidViscosity(T, p);
        }

        assert (phaseIdx == gPhaseIdx);
        
        /* Wilke method. See:
         *
         * See: R. Reid, et al.: The Properties of Gases and Liquids,
         * 4th edition, McGraw-Hill, 1987, 407-410
         * 5th edition, McGraw-Hill, 20001, p. 9.21/22
         *
         * in this case, we use a simplified version in order to avoid
         * computationally costly evaluation of sqrt and pow functions and
         * divisions
         * -- compare e.g. with Promo Class p. 32/33
         */
        Scalar muResult;
        const Scalar mu[numComponents] = {
            H2O::gasViscosity(T, H2O::vaporPressure(T)),
            Air::gasViscosity(T, p),
            NAPL::gasViscosity(T, NAPL::vaporPressure(T))
        };
        // molar masses
        const Scalar M[numComponents] = {
            H2O::molarMass(),
            Air::molarMass(),
            NAPL::molarMass()
        };

        Scalar muAW = mu[airIdx]*fluidState.moleFraction(gPhaseIdx, airIdx)
            + mu[H2OIdx]*fluidState.moleFraction(gPhaseIdx, H2OIdx)
            / (fluidState.moleFraction(gPhaseIdx, airIdx)
               + fluidState.moleFraction(gPhaseIdx, H2OIdx));
        Scalar xAW = fluidState.moleFraction(gPhaseIdx, airIdx)
            + fluidState.moleFraction(gPhaseIdx, H2OIdx);

        Scalar MAW = (fluidState.moleFraction(gPhaseIdx, airIdx)*Air::molarMass()
                      + fluidState.moleFraction(gPhaseIdx, H2OIdx)*H2O::molarMass())
            / xAW;

        Scalar phiCAW = 0.3; // simplification for this particular system
        /* actually like this
         * Scalar phiCAW = std::pow(1.+std::sqrt(mu[NAPLIdx]/muAW)*std::pow(MAW/M[NAPLIdx],0.25),2)
         *                 / std::sqrt(8.*(1.+M[NAPLIdx]/MAW));
         */
        Scalar phiAWC = phiCAW * muAW*M[NAPLIdx]/(mu[NAPLIdx]*MAW);

        muResult = (xAW*muAW)/(xAW+fluidState.moleFraction(gPhaseIdx, NAPLIdx)*phiAWC)
            + (fluidState.moleFraction(gPhaseIdx, NAPLIdx) * mu[NAPLIdx])
            / (fluidState.moleFraction(gPhaseIdx, NAPLIdx) + xAW*phiCAW);
        return muResult;
    }


    /*!
     * \brief Given all mole fractions, return the diffusion
     *        coefficent of a component in a phase.
     */
    template <class FluidState>
    static Scalar diffusionCoefficient(const FluidState &fluidState,
                                       const ParameterCache &paramCache,
                                       int phaseIdx,
                                       int compIdx)
    {
        return 0;
#if 0
        Scalar T = fluidState.temperature(phaseIdx) ;
        Scalar p = fluidState.pressure(phaseIdx);
        Scalar diffCont;

        if (phaseIdx==gPhaseIdx) {
            Scalar diffAC = Dumux::BinaryCoeff::Air_Mesitylene::gasDiffCoeff(T, p);
            Scalar diffWC = Dumux::BinaryCoeff::H2O_Mesitylene::gasDiffCoeff(T, p);
            Scalar diffAW = Dumux::BinaryCoeff::H2O_Air::gasDiffCoeff(T, p);

            const Scalar xga = fluidState.moleFraction(gPhaseIdx, airIdx);
            const Scalar xgw = fluidState.moleFraction(gPhaseIdx, H2OIdx);
            const Scalar xgc = fluidState.moleFraction(gPhaseIdx, NAPLIdx);

            if (compIdx==NAPLIdx) return (1 - xgw)/(xga/diffAW + xgc/diffWC);
            else if (compIdx==H2OIdx) return (1 - xgc)/(xgw/diffWC + xga/diffAC);
            else if (compIdx==airIdx) DUNE_THROW(Dune::InvalidStateException,
                                                 "Diffusivity of air in the gas phase "
                                                 "is constraint by sum of diffusive fluxes = 0 !\n");
        }
        else if (phaseIdx==wPhaseIdx){
            Scalar diffACl = 1.e-9; // BinaryCoeff::Air_Mesitylene::liquidDiffCoeff(temperature, pressure);
            Scalar diffWCl = 1.e-9; // BinaryCoeff::H2O_Mesitylene::liquidDiffCoeff(temperature, pressure);
            Scalar diffAWl = 1.e-9; // BinaryCoeff::H2O_Air::liquidDiffCoeff(temperature, pressure);

            Scalar xwa = fluidState.moleFraction(wPhaseIdx, airIdx);
            Scalar xww = fluidState.moleFraction(wPhaseIdx, H2OIdx);
            Scalar xwc = fluidState.moleFraction(wPhaseIdx, NAPLIdx);

            switch (compIdx) {
            case NAPLIdx:
                diffCont = (1.- xww)/(xwa/diffAWl + xwc/diffWCl);
                return diffCont;
            case airIdx:
                diffCont = (1.- xwc)/(xww/diffWCl + xwa/diffACl);
                return diffCont;
            case H2OIdx:
                DUNE_THROW(Dune::InvalidStateException,
                           "Diffusivity of water in the water phase "
                           "is constraint by sum of diffusive fluxes = 0 !\n");
            };
        }
        else if (phaseIdx==nPhaseIdx) {
            DUNE_THROW(Dune::InvalidStateException,
                       "Diffusion coefficients of "
                       "substances in liquid phase are undefined!\n");
        }
        return 0;
#endif
    }

    /*!
     * \brief Returns the fugacity coefficient [-] of a component in a
     *        phase.
     *
     * In this case, things are actually pretty simple. We have an ideal
     * solution. Thus, the fugacity coefficient is 1 in the gas phase
     * (fugacity equals the partial pressure of the component in the gas phase
     * respectively in the liquid phases it is the inverse of the
     * Henry coefficients scaled by pressure
     */
    template <class FluidState>
    static Scalar fugacityCoefficient(const FluidState &fluidState,
                                      const ParameterCache &paramCache,
                                      int phaseIdx,
                                      int compIdx)
    {
        assert(0 <= phaseIdx  && phaseIdx < numPhases);
        assert(0 <= compIdx  && compIdx < numComponents);

        Scalar T = fluidState.temperature(phaseIdx);
        Scalar p = fluidState.pressure(phaseIdx);
        Valgrind::CheckDefined(T);
        Valgrind::CheckDefined(p);

        if (phaseIdx == wPhaseIdx) {
            if (compIdx == H2OIdx)
                return H2O::vaporPressure(T)/p;
            else if (compIdx == airIdx)
                return Dumux::BinaryCoeff::H2O_N2::henry(T)/p;
            else if (compIdx == NAPLIdx)
                return Dumux::BinaryCoeff::H2O_Mesitylene::henry(T)/p;
            assert(false);
        }
        // for the NAPL phase, we assume currently that nothing is
        // dissolved. this means that the affinity of the NAPL
        // component to the NAPL phase is much higher than for the
        // other components, i.e. the fugacity cofficient is much
        // smaller.
        else if (phaseIdx == nPhaseIdx) {
            Scalar phiNapl = NAPL::vaporPressure(T)/p;
            if (compIdx == NAPLIdx)
                return phiNapl;
            else if (compIdx == airIdx)
                return 1e6*phiNapl;
            else if (compIdx == H2OIdx)
                return 1e6*phiNapl;
            assert(false);
        }

        // for the gas phase, assume an ideal gas when it comes to
        // fugacity (-> fugacity == partial pressure)
        assert(phaseIdx == gPhaseIdx);
        return 1.0;
    }


    /*!
     * \brief Given all mole fractions in a phase, return the specific
     *        phase enthalpy [J/kg].
     *
     * \todo This system neglects the contribution of gas-molecules in
     *       the liquid phase.  This contribution is probably not
     *       big. Somebody would have to find out the enthalpy of
     *       solution for this system. ...
     */
    template <class FluidState>
    static Scalar enthalpy(const FluidState &fluidState,
                           const ParameterCache &paramCache,
                           int phaseIdx)
    {
        Scalar T = fluidState.temperature(phaseIdx) ;
        Scalar p = fluidState.pressure(phaseIdx);

        if (phaseIdx == wPhaseIdx) {
            return H2O::liquidEnthalpy(T, p);
        }
        else if (phaseIdx == nPhaseIdx) {
            return NAPL::liquidEnthalpy(T, p);
        }
        else if (phaseIdx == gPhaseIdx) {
            // gas phase enthalpy depends strongly on composition
            Scalar result = 0;
            result += H2O::gasEnthalpy(T, p) * fluidState.massFraction(gPhaseIdx, H2OIdx);
            result += NAPL::gasEnthalpy(T, p) * fluidState.massFraction(gPhaseIdx, airIdx);
            result += Air::gasEnthalpy(T, p) * fluidState.massFraction(gPhaseIdx, NAPLIdx);

            return result;
        }
        DUNE_THROW(Dune::InvalidStateException, "Invalid phase index " << phaseIdx);
    }
    
    /*!
     * \brief Thermal conductivity of a fluid phase [W/(m K)].
     *
     * Use the conductivity of air and water as a approximation.
     *
     * \param fluidState An abitrary fluid state
     * \param phaseIdx The index of the fluid phase to consider
     */
    template <class FluidState>
    static Scalar thermalConductivity(const FluidState &fluidState,
                                      const ParameterCache &paramCache,
                                      int phaseIdx)
    {
        assert(0 <= phaseIdx  && phaseIdx < numPhases);

        Scalar T = fluidState.temperature(phaseIdx) ;
        Scalar p = fluidState.pressure(phaseIdx);

        if (phaseIdx == wPhaseIdx){ // water phase
            return H2O::liquidThermalConductivity(T, p);
        }
        else if (phaseIdx == gPhaseIdx) { // gas phase
            Scalar lambdaDryAir = Air::gasThermalConductivity(T, p);
            return lambdaDryAir;
        }

        assert(phaseIdx == nPhaseIdx);
        
        // Taken from: 
        //
        // D. K. H. Briggs: "Thermal Conductivity of Liquids",
        // Ind. Eng. Chem., 1957, 49 (3), pp 418–421
        //
        // Convertion to SI units: 
        // 344e-6 cal/(s cm K) = 0.0143964 J/(s m K)
        return 0.0143964;
    }
};
} // end namespace FluidSystems
} // end namespace Dumux

#endif
