<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.6.2">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="2.54" unitdist="mm" unit="mm" style="lines" multiple="1" display="no" altdistance="0.254" altunitdist="mm" altunit="mm"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="yes" active="yes"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="yes" active="yes"/>
<layer number="17" name="Pads" color="2" fill="1" visible="yes" active="yes"/>
<layer number="18" name="Vias" color="2" fill="1" visible="yes" active="yes"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="yes" active="yes"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="yes" active="yes"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="yes" active="yes"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="yes" active="yes"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="yes" active="yes"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="yes" active="yes"/>
<layer number="25" name="tNames" color="7" fill="1" visible="yes" active="yes"/>
<layer number="26" name="bNames" color="7" fill="1" visible="yes" active="yes"/>
<layer number="27" name="tValues" color="7" fill="1" visible="yes" active="yes"/>
<layer number="28" name="bValues" color="7" fill="1" visible="yes" active="yes"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="yes"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="yes"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="yes"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="yes"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="yes" active="yes"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="yes" active="yes"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="yes" active="yes"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="yes" active="yes"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="yes" active="yes"/>
<layer number="44" name="Drills" color="7" fill="9" visible="no" active="yes"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="yes"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="yes"/>
<layer number="48" name="Document" color="7" fill="1" visible="yes" active="yes"/>
<layer number="49" name="Reference" color="7" fill="1" visible="yes" active="yes"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="yes" active="yes"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="yes" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="99" name="SpiceOrder" color="7" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="MotoRecoPico">
<description>MotoRecoPico - Raspberry Pi Pico 2 based CAN logger custom parts</description>
<packages>
<package name="MQS-8-RA-967658">
<description>TE 1-967658-1 MQS .63 automotive header, 8 position, 2 rows, right angle, through hole.
Origin is at the housing front reference line (drawing datum), on the connector centreline.
Board edge should be placed at Y=0. Holes are at negative Y (into the board).
Front hole row Y=-3.76: pins 4,3,2,1 at X=-5,-1,+3,+7
Rear  hole row Y=-6.30: pins 8,7,6,5 at X=-7,-3,+1,+5
Drawing 967658 Rev.C3. Hole spec is 1.0mm minimum; 1.1mm drill used.
Silkscreen body outline is indicative only - hole positions are the controlled dimensions.</description>
<pad name="1" x="7" y="-3.76" drill="1.1" diameter="1.8"/>
<pad name="2" x="3" y="-3.76" drill="1.1" diameter="1.8"/>
<pad name="3" x="-1" y="-3.76" drill="1.1" diameter="1.8"/>
<pad name="4" x="-5" y="-3.76" drill="1.1" diameter="1.8"/>
<pad name="5" x="5" y="-6.3" drill="1.1" diameter="1.8"/>
<pad name="6" x="1" y="-6.3" drill="1.1" diameter="1.8"/>
<pad name="7" x="-3" y="-6.3" drill="1.1" diameter="1.8"/>
<pad name="8" x="-7" y="-6.3" drill="1.1" diameter="1.8"/>
<wire x1="-9.8" y1="0" x2="9.8" y2="0" width="0.15" layer="21"/>
<wire x1="9.8" y1="0" x2="9.8" y2="-8" width="0.15" layer="21"/>
<wire x1="9.8" y1="-8" x2="-9.8" y2="-8" width="0.15" layer="21"/>
<wire x1="-9.8" y1="-8" x2="-9.8" y2="0" width="0.15" layer="21"/>
<wire x1="-9.8" y1="0" x2="9.8" y2="0" width="0.2" layer="51"/>
<circle x="8.8" y="-2.2" radius="0.3" width="0.3" layer="21"/>
<text x="-9.8" y="0.5" size="1.016" layer="25">&gt;NAME</text>
<text x="-9.8" y="-10" size="1.016" layer="27">&gt;VALUE</text>
</package>
<package name="SSOP28">
<description>28-lead SSOP, 5.30mm body, 0.65mm pitch (Microchip package code SS).
Land pattern per Microchip C04-073: contact pad spacing C=7.20mm,
pad width X=0.40mm, pad length Y=1.75mm.
Hand-solderable with care.</description>
<smd name="1" x="-3.6" y="4.225" dx="1.75" dy="0.4" layer="1"/>
<smd name="2" x="-3.6" y="3.575" dx="1.75" dy="0.4" layer="1"/>
<smd name="3" x="-3.6" y="2.925" dx="1.75" dy="0.4" layer="1"/>
<smd name="4" x="-3.6" y="2.275" dx="1.75" dy="0.4" layer="1"/>
<smd name="5" x="-3.6" y="1.625" dx="1.75" dy="0.4" layer="1"/>
<smd name="6" x="-3.6" y="0.975" dx="1.75" dy="0.4" layer="1"/>
<smd name="7" x="-3.6" y="0.325" dx="1.75" dy="0.4" layer="1"/>
<smd name="8" x="-3.6" y="-0.325" dx="1.75" dy="0.4" layer="1"/>
<smd name="9" x="-3.6" y="-0.975" dx="1.75" dy="0.4" layer="1"/>
<smd name="10" x="-3.6" y="-1.625" dx="1.75" dy="0.4" layer="1"/>
<smd name="11" x="-3.6" y="-2.275" dx="1.75" dy="0.4" layer="1"/>
<smd name="12" x="-3.6" y="-2.925" dx="1.75" dy="0.4" layer="1"/>
<smd name="13" x="-3.6" y="-3.575" dx="1.75" dy="0.4" layer="1"/>
<smd name="14" x="-3.6" y="-4.225" dx="1.75" dy="0.4" layer="1"/>
<smd name="15" x="3.6" y="-4.225" dx="1.75" dy="0.4" layer="1"/>
<smd name="16" x="3.6" y="-3.575" dx="1.75" dy="0.4" layer="1"/>
<smd name="17" x="3.6" y="-2.925" dx="1.75" dy="0.4" layer="1"/>
<smd name="18" x="3.6" y="-2.275" dx="1.75" dy="0.4" layer="1"/>
<smd name="19" x="3.6" y="-1.625" dx="1.75" dy="0.4" layer="1"/>
<smd name="20" x="3.6" y="-0.975" dx="1.75" dy="0.4" layer="1"/>
<smd name="21" x="3.6" y="-0.325" dx="1.75" dy="0.4" layer="1"/>
<smd name="22" x="3.6" y="0.325" dx="1.75" dy="0.4" layer="1"/>
<smd name="23" x="3.6" y="0.975" dx="1.75" dy="0.4" layer="1"/>
<smd name="24" x="3.6" y="1.625" dx="1.75" dy="0.4" layer="1"/>
<smd name="25" x="3.6" y="2.275" dx="1.75" dy="0.4" layer="1"/>
<smd name="26" x="3.6" y="2.925" dx="1.75" dy="0.4" layer="1"/>
<smd name="27" x="3.6" y="3.575" dx="1.75" dy="0.4" layer="1"/>
<smd name="28" x="3.6" y="4.225" dx="1.75" dy="0.4" layer="1"/>
<wire x1="-2.65" y1="5.1" x2="2.65" y2="5.1" width="0.15" layer="21"/>
<wire x1="2.65" y1="5.1" x2="2.65" y2="-5.1" width="0.15" layer="21"/>
<wire x1="2.65" y1="-5.1" x2="-2.65" y2="-5.1" width="0.15" layer="21"/>
<wire x1="-2.65" y1="-5.1" x2="-2.65" y2="5.1" width="0.15" layer="21"/>
<circle x="-1.9" y="4.35" radius="0.35" width="0.3" layer="21"/>
<text x="-2.65" y="5.6" size="1.016" layer="25">&gt;NAME</text>
<text x="-2.65" y="-6.6" size="1.016" layer="27">&gt;VALUE</text>
</package>
<package name="RPI-PICO2-THT">
<description>Raspberry Pi Pico 2 (RP2350) mounted as a module on a carrier board.
2 x 20 through holes, 2.54mm pitch, 17.78mm (0.7 inch) row spacing.
Origin is the centre of the hole grid. Pin 1 (GP0) and pin 40 (VBUS) are at the USB end (+Y).
Hole grid is the controlled geometry; the 21 x 51mm board outline on tPlace is indicative.
Drill 1.0mm suits 0.1 inch header pins - use a socket strip if the module must stay removable.</description>
<pad name="1" x="-8.89" y="24.13" drill="1" diameter="1.7"/>
<pad name="2" x="-8.89" y="21.59" drill="1" diameter="1.7"/>
<pad name="3" x="-8.89" y="19.05" drill="1" diameter="1.7"/>
<pad name="4" x="-8.89" y="16.51" drill="1" diameter="1.7"/>
<pad name="5" x="-8.89" y="13.97" drill="1" diameter="1.7"/>
<pad name="6" x="-8.89" y="11.43" drill="1" diameter="1.7"/>
<pad name="7" x="-8.89" y="8.89" drill="1" diameter="1.7"/>
<pad name="8" x="-8.89" y="6.35" drill="1" diameter="1.7"/>
<pad name="9" x="-8.89" y="3.81" drill="1" diameter="1.7"/>
<pad name="10" x="-8.89" y="1.27" drill="1" diameter="1.7"/>
<pad name="11" x="-8.89" y="-1.27" drill="1" diameter="1.7"/>
<pad name="12" x="-8.89" y="-3.81" drill="1" diameter="1.7"/>
<pad name="13" x="-8.89" y="-6.35" drill="1" diameter="1.7"/>
<pad name="14" x="-8.89" y="-8.89" drill="1" diameter="1.7"/>
<pad name="15" x="-8.89" y="-11.43" drill="1" diameter="1.7"/>
<pad name="16" x="-8.89" y="-13.97" drill="1" diameter="1.7"/>
<pad name="17" x="-8.89" y="-16.51" drill="1" diameter="1.7"/>
<pad name="18" x="-8.89" y="-19.05" drill="1" diameter="1.7"/>
<pad name="19" x="-8.89" y="-21.59" drill="1" diameter="1.7"/>
<pad name="20" x="-8.89" y="-24.13" drill="1" diameter="1.7"/>
<pad name="21" x="8.89" y="-24.13" drill="1" diameter="1.7"/>
<pad name="22" x="8.89" y="-21.59" drill="1" diameter="1.7"/>
<pad name="23" x="8.89" y="-19.05" drill="1" diameter="1.7"/>
<pad name="24" x="8.89" y="-16.51" drill="1" diameter="1.7"/>
<pad name="25" x="8.89" y="-13.97" drill="1" diameter="1.7"/>
<pad name="26" x="8.89" y="-11.43" drill="1" diameter="1.7"/>
<pad name="27" x="8.89" y="-8.89" drill="1" diameter="1.7"/>
<pad name="28" x="8.89" y="-6.35" drill="1" diameter="1.7"/>
<pad name="29" x="8.89" y="-3.81" drill="1" diameter="1.7"/>
<pad name="30" x="8.89" y="-1.27" drill="1" diameter="1.7"/>
<pad name="31" x="8.89" y="1.27" drill="1" diameter="1.7"/>
<pad name="32" x="8.89" y="3.81" drill="1" diameter="1.7"/>
<pad name="33" x="8.89" y="6.35" drill="1" diameter="1.7"/>
<pad name="34" x="8.89" y="8.89" drill="1" diameter="1.7"/>
<pad name="35" x="8.89" y="11.43" drill="1" diameter="1.7"/>
<pad name="36" x="8.89" y="13.97" drill="1" diameter="1.7"/>
<pad name="37" x="8.89" y="16.51" drill="1" diameter="1.7"/>
<pad name="38" x="8.89" y="19.05" drill="1" diameter="1.7"/>
<pad name="39" x="8.89" y="21.59" drill="1" diameter="1.7"/>
<pad name="40" x="8.89" y="24.13" drill="1" diameter="1.7"/>
<wire x1="-10.5" y1="25.5" x2="10.5" y2="25.5" width="0.15" layer="21"/>
<wire x1="10.5" y1="25.5" x2="10.5" y2="-25.5" width="0.15" layer="21"/>
<wire x1="10.5" y1="-25.5" x2="-10.5" y2="-25.5" width="0.15" layer="21"/>
<wire x1="-10.5" y1="-25.5" x2="-10.5" y2="25.5" width="0.15" layer="21"/>
<wire x1="-4" y1="25.5" x2="4" y2="25.5" width="0.6" layer="21"/>
<circle x="-11.4" y="24.13" radius="0.35" width="0.3" layer="21"/>
<text x="-4" y="23" size="1.016" layer="21" align="top-center">USB</text>
<text x="-10.5" y="26" size="1.27" layer="25">&gt;NAME</text>
<text x="-10.5" y="-27.5" size="1.27" layer="27">&gt;VALUE</text>
</package>
<package name="MICROSD-DM3AT">
<description>Hirose DM3AT-SF-PEJM5 microSD socket, push-push, SMT right angle.
Land pattern taken from the Hirose official 2D drawing ADC-325165-00-01
(recommended PCB pattern, mounting-surface side view).
Scale was recovered from the drawing vector geometry and cross-checked against four
independent dimensions: pitch 1.1, 8-pad span 7.7, pad width 0.7, pad height 1.2.
Signal pads 1-8: 0.70 x 1.20mm, 1.1mm pitch, pad 1 at X=+3.85 (next to pad TR).
Six auxiliary pads are named by position, not by function - see the deviceset
description. Confirm with a continuity check on the physical part before ordering.
Keep-out (hatched) regions from the drawing are NOT yet modelled - do not flood
copper close to the body until they are added.</description>
<smd name="1" x="3.85" y="0" dx="0.7" dy="1.2" layer="1"/>
<smd name="2" x="2.75" y="0" dx="0.7" dy="1.2" layer="1"/>
<smd name="3" x="1.65" y="0" dx="0.7" dy="1.2" layer="1"/>
<smd name="4" x="0.55" y="0" dx="0.7" dy="1.2" layer="1"/>
<smd name="5" x="-0.55" y="0" dx="0.7" dy="1.2" layer="1"/>
<smd name="6" x="-1.65" y="0" dx="0.7" dy="1.2" layer="1"/>
<smd name="7" x="-2.75" y="0" dx="0.7" dy="1.2" layer="1"/>
<smd name="8" x="-3.85" y="0" dx="0.7" dy="1.2" layer="1"/>
<smd name="TR" x="5.4" y="0" dx="1" dy="1.2" layer="1"/>
<smd name="TL" x="-4.8" y="0" dx="0.7" dy="1.2" layer="1"/>
<smd name="L1" x="-5.75" y="-4.3" dx="1" dy="1.2" layer="1"/>
<smd name="L2" x="-5.75" y="-10.5" dx="1" dy="0.8" layer="1"/>
<smd name="L3" x="-5.75" y="-14.65" dx="1" dy="2.8" layer="1"/>
<smd name="BR" x="5.27" y="-15.37" dx="2.55" dy="1.36" layer="1"/>
<wire x1="-7" y1="1" x2="7" y2="1" width="0.15" layer="51"/>
<wire x1="7" y1="1" x2="7" y2="-15" width="0.15" layer="51"/>
<wire x1="7" y1="-15" x2="-7" y2="-15" width="0.15" layer="51"/>
<wire x1="-7" y1="-15" x2="-7" y2="1" width="0.15" layer="51"/>
<circle x="+3.85" y="1.6" radius="0.3" width="0.3" layer="21"/>
<text x="-7" y="2.2" size="1.016" layer="25">&gt;NAME</text>
<text x="-7" y="-17" size="1.016" layer="27">&gt;VALUE</text>
</package>
<package name="C1005">
<description>Chip resistor / capacitor, 1005 metric (1.0 x 0.5mm, imperial 0402).
Reflow land pattern for assembly-house placement (JLCPCB), not a hand-solder land:
pads 0.59 x 0.64mm on 0.97mm centres, giving a 0.38mm inner gap.
JLCPCB places 0402 as standard, so no derating is needed.</description>
<smd name="1" x="-0.485" y="0" dx="0.59" dy="0.64" layer="1"/>
<smd name="2" x="0.485" y="0" dx="0.59" dy="0.64" layer="1"/>
<wire x1="-0.9" y1="-0.42" x2="0.9" y2="-0.42" width="0.1" layer="21"/>
<wire x1="0.9" y1="-0.42" x2="0.9" y2="0.42" width="0.1" layer="21"/>
<wire x1="0.9" y1="0.42" x2="-0.9" y2="0.42" width="0.1" layer="21"/>
<wire x1="-0.9" y1="0.42" x2="-0.9" y2="-0.42" width="0.1" layer="21"/>
<text x="-0.9" y="0.9" size="0.6" layer="25">&gt;NAME</text>
<text x="-0.9" y="-1.5" size="0.6" layer="27">&gt;VALUE</text>
</package>
<package name="C1608">
<description>Chip capacitor, 1608 metric (1.6 x 0.8mm, imperial 0603).
Used only for the two 10uF bulk capacitors: 10uF is not practically available
in 1005 (and is not a JLCPCB basic part in that size), so those two stay 1608.
Reflow land: pads 0.9 x 0.95mm on 1.55mm centres.</description>
<smd name="1" x="-0.775" y="0" dx="0.9" dy="0.95" layer="1"/>
<smd name="2" x="0.775" y="0" dx="0.9" dy="0.95" layer="1"/>
<wire x1="-1.4" y1="-0.6" x2="1.4" y2="-0.6" width="0.1" layer="21"/>
<wire x1="1.4" y1="-0.6" x2="1.4" y2="0.6" width="0.1" layer="21"/>
<wire x1="1.4" y1="0.6" x2="-1.4" y2="0.6" width="0.1" layer="21"/>
<wire x1="-1.4" y1="0.6" x2="-1.4" y2="-0.6" width="0.1" layer="21"/>
<text x="-1.4" y="1.1" size="0.7" layer="25">&gt;NAME</text>
<text x="-1.4" y="-1.9" size="0.7" layer="27">&gt;VALUE</text>
</package>
<package name="C1206">
<description>2-terminal 1206 (3216 metric) chip land. Used for the PTC fuse.</description>
<smd name="1" x="-1.5" y="0" dx="1.6" dy="1.8" layer="1"/>
<smd name="2" x="1.5" y="0" dx="1.6" dy="1.8" layer="1"/>
<wire x1="-2.4" y1="-1.1" x2="2.4" y2="-1.1" width="0.15" layer="21"/>
<wire x1="2.4" y1="-1.1" x2="2.4" y2="1.1" width="0.15" layer="21"/>
<wire x1="2.4" y1="1.1" x2="-2.4" y2="1.1" width="0.15" layer="21"/>
<wire x1="-2.4" y1="1.1" x2="-2.4" y2="-1.1" width="0.15" layer="21"/>
<text x="-2.4" y="1.6" size="1.016" layer="25">&gt;NAME</text>
<text x="-2.4" y="-3.2" size="1.016" layer="27">&gt;VALUE</text>
</package>
<package name="SMA">
<description>DO-214AC (SMA). Pin 1 = cathode (banded end).</description>
<smd name="1" x="-2.15" y="0" dx="2.4" dy="1.75" layer="1"/>
<smd name="2" x="2.15" y="0" dx="2.4" dy="1.75" layer="1"/>
<wire x1="-2.2" y1="-1.4" x2="2.2" y2="-1.4" width="0.15" layer="21"/>
<wire x1="2.2" y1="-1.4" x2="2.2" y2="1.4" width="0.15" layer="21"/>
<wire x1="2.2" y1="1.4" x2="-2.2" y2="1.4" width="0.15" layer="21"/>
<wire x1="-2.2" y1="1.4" x2="-2.2" y2="-1.4" width="0.15" layer="21"/>
<wire x1="-1.3" y1="-1.4" x2="-1.3" y2="1.4" width="0.3" layer="21"/>
<text x="-2.6" y="2" size="1.016" layer="25">&gt;NAME</text>
<text x="-2.6" y="-4" size="1.016" layer="27">&gt;VALUE</text>
</package>
<package name="SOD123">
<description>SOD-123. Pin 1 = cathode (banded end).</description>
<smd name="1" x="-1.65" y="0" dx="1" dy="1.2" layer="1"/>
<smd name="2" x="1.65" y="0" dx="1" dy="1.2" layer="1"/>
<wire x1="-1.4" y1="-0.9" x2="1.4" y2="-0.9" width="0.15" layer="21"/>
<wire x1="1.4" y1="-0.9" x2="1.4" y2="0.9" width="0.15" layer="21"/>
<wire x1="1.4" y1="0.9" x2="-1.4" y2="0.9" width="0.15" layer="21"/>
<wire x1="-1.4" y1="0.9" x2="-1.4" y2="-0.9" width="0.15" layer="21"/>
<wire x1="-0.8" y1="-0.9" x2="-0.8" y2="0.9" width="0.25" layer="21"/>
<text x="-1.8" y="1.4" size="1.016" layer="25">&gt;NAME</text>
<text x="-1.8" y="-2.8" size="1.016" layer="27">&gt;VALUE</text>
</package>
<package name="SOT23">
<description>SOT-23 (TO-236AB). Pins 1,2 on one side, pin 3 opposite.</description>
<smd name="1" x="-0.95" y="-1.1" dx="0.9" dy="1" layer="1"/>
<smd name="2" x="0.95" y="-1.1" dx="0.9" dy="1" layer="1"/>
<smd name="3" x="0" y="1.1" dx="0.9" dy="1" layer="1"/>
<wire x1="-1.5" y1="-0.7" x2="1.5" y2="-0.7" width="0.15" layer="21"/>
<wire x1="1.5" y1="-0.7" x2="1.5" y2="0.7" width="0.15" layer="21"/>
<wire x1="1.5" y1="0.7" x2="-1.5" y2="0.7" width="0.15" layer="21"/>
<wire x1="-1.5" y1="0.7" x2="-1.5" y2="-0.7" width="0.15" layer="21"/>
<circle x="-1.7" y="-1.1" radius="0.2" width="0.25" layer="21"/>
<text x="-1.6" y="2" size="1.016" layer="25">&gt;NAME</text>
<text x="-1.6" y="-4" size="1.016" layer="27">&gt;VALUE</text>
</package>
<package name="HC49S">
<description>HC-49/S through-hole crystal, 4.88mm lead pitch.</description>
<pad name="1" x="-2.44" y="0" drill="0.8" diameter="1.5"/>
<pad name="2" x="2.44" y="0" drill="0.8" diameter="1.5"/>
<wire x1="-5.5" y1="-1.8" x2="5.5" y2="-1.8" width="0.15" layer="21"/>
<wire x1="5.5" y1="-1.8" x2="5.5" y2="1.8" width="0.15" layer="21"/>
<wire x1="5.5" y1="1.8" x2="-5.5" y2="1.8" width="0.15" layer="21"/>
<wire x1="-5.5" y1="1.8" x2="-5.5" y2="-1.8" width="0.15" layer="21"/>
<text x="-5.5" y="2.3" size="1.016" layer="25">&gt;NAME</text>
<text x="-5.5" y="-4.6" size="1.016" layer="27">&gt;VALUE</text>
</package>
<package name="SJ2">
<description>Solder jumper, 2 pads. Bridge with solder to close.
For JP1 the DEFAULT STATE IS CLOSED (CAN termination active).</description>
<smd name="1" x="-0.75" y="0" dx="1" dy="1.5" layer="1"/>
<smd name="2" x="0.75" y="0" dx="1" dy="1.5" layer="1"/>
<wire x1="-1.6" y1="-1.1" x2="1.6" y2="-1.1" width="0.15" layer="21"/>
<wire x1="1.6" y1="-1.1" x2="1.6" y2="1.1" width="0.15" layer="21"/>
<wire x1="1.6" y1="1.1" x2="-1.6" y2="1.1" width="0.15" layer="21"/>
<wire x1="-1.6" y1="1.1" x2="-1.6" y2="-1.1" width="0.15" layer="21"/>
<text x="-1.6" y="1.6" size="1.016" layer="25">&gt;NAME</text>
<text x="-1.6" y="-3.2" size="1.016" layer="27">&gt;VALUE</text>
</package>
<package name="LED-SIDE-2812">
<description>Side-view chip LED, 2.8 x 1.2 x 0.8mm body.
Recommended soldering pattern from both the Kodenshi LP812-010(T) and the
Sharp GM4ZR83200AE datasheets: two 1.4 x 0.9mm pads with a 1.0mm gap,
so pad centres sit at X = +/-1.2mm. Pad 1 = CATHODE, pad 2 = ANODE.
Light leaves through one long side - point that side at the enclosure window
and place the part on the board edge.</description>
<smd name="1" x="-1.2" y="0" dx="1.4" dy="0.9" layer="1"/>
<smd name="2" x="1.2" y="0" dx="1.4" dy="0.9" layer="1"/>
<wire x1="-1.4" y1="-0.6" x2="1.4" y2="-0.6" width="0.15" layer="21"/>
<wire x1="1.4" y1="-0.6" x2="1.4" y2="0.6" width="0.15" layer="21"/>
<wire x1="1.4" y1="0.6" x2="-1.4" y2="0.6" width="0.15" layer="21"/>
<wire x1="-1.4" y1="0.6" x2="-1.4" y2="-0.6" width="0.15" layer="21"/>
<wire x1="-1.4" y1="-0.6" x2="-1.4" y2="0.6" width="0.3" layer="21"/>
<wire x1="-1.6" y1="0.9" x2="1.6" y2="0.9" width="0.3" layer="51"/>
<text x="-1.8" y="1.4" size="1.016" layer="25">&gt;NAME</text>
<text x="-1.8" y="-2.8" size="1.016" layer="27">&gt;VALUE</text>
</package>
<package name="SIP3-M78AR05">
<description>MinMax M78AR05-0.5 switching regulator, SIP-3.
Pins on 2 x 2.54mm pitch, LM78xx-compatible: 1 = +Vin, 2 = GND, 3 = +Vout.
Pin cross-section 0.70 x 0.25mm, so a 1.0mm drill clears it with margin.
Body 11.5 x 7.55 x 10.2mm, pin row centred along the 11.5mm dimension.
Origin = pin 2 (GND). Silkscreen is indicative; the holes are the controlled geometry.
This is the tallest part after J1 - keep it on the same side as J1.</description>
<pad name="1" x="-2.54" y="0" drill="1" diameter="1.8" shape="square"/>
<pad name="2" x="0" y="0" drill="1" diameter="1.8"/>
<pad name="3" x="2.54" y="0" drill="1" diameter="1.8"/>
<wire x1="-5.75" y1="-2" x2="5.75" y2="-2" width="0.15" layer="21"/>
<wire x1="5.75" y1="-2" x2="5.75" y2="5.55" width="0.15" layer="21"/>
<wire x1="5.75" y1="5.55" x2="-5.75" y2="5.55" width="0.15" layer="21"/>
<wire x1="-5.75" y1="5.55" x2="-5.75" y2="-2" width="0.15" layer="21"/>
<text x="-4.4" y="-1.6" size="0.8128" layer="21">Vin</text>
<text x="1.6" y="-1.6" size="0.8128" layer="21">Vout</text>
<text x="-5.75" y="6" size="1.016" layer="25">&gt;NAME</text>
<text x="-5.75" y="-4" size="1.016" layer="27">&gt;VALUE</text>
</package>
<package name="GPS-5P-SMD">
<description>GT-502MGG-N bare-wire landing pads, surface mount, single sided.
Five 2.0 x 3.0mm pads on 2.54mm pitch. No through holes, so nothing is blocked
on the opposite side and the wires can only be dressed from this face.
Pin order per datasheet: 1 VCC, 2 GND, 3 TXD, 4 RXD, 5 PPS.
Strain relief is the enclosure's job (cable clamp or gland) - there are no
cable-tie holes, deliberately, to save board area.</description>
<smd name="1" x="-5.08" y="0" dx="2" dy="3" layer="1"/>
<smd name="2" x="-2.54" y="0" dx="2" dy="3" layer="1"/>
<smd name="3" x="0" y="0" dx="2" dy="3" layer="1"/>
<smd name="4" x="2.54" y="0" dx="2" dy="3" layer="1"/>
<smd name="5" x="5.08" y="0" dx="2" dy="3" layer="1"/>
<text x="-5.08" y="2" size="0.8128" layer="21" align="bottom-center">V</text>
<text x="-2.54" y="2" size="0.8128" layer="21" align="bottom-center">G</text>
<text x="0" y="2" size="0.8128" layer="21" align="bottom-center">T</text>
<text x="2.54" y="2" size="0.8128" layer="21" align="bottom-center">R</text>
<text x="5.08" y="2" size="0.8128" layer="21" align="bottom-center">P</text>
<wire x1="-6.5" y1="-1.9" x2="6.5" y2="-1.9" width="0.15" layer="21"/>
<text x="-6.5" y="3.4" size="1.016" layer="25">&gt;NAME</text>
<text x="-6.5" y="-4" size="1.016" layer="27">&gt;VALUE</text>
</package>
<package name="SOT26">
<description>SOT-26 / SOT-23-6, 0.95mm pitch, land rows 2.6mm centre-to-centre.
Pin 1 is the bottom-left pad, numbering runs counter-clockwise.
For ZXMP6A17E6Q: pad 1 = Gate, pad 6 = Source, pads 2-5 = Drain (four drain
pads form the thermal path and are all one terminal).</description>
<smd name="1" x="-1.3" y="-0.95" dx="1.1" dy="0.6" layer="1"/>
<smd name="2" x="-1.3" y="0" dx="1.1" dy="0.6" layer="1"/>
<smd name="3" x="-1.3" y="0.95" dx="1.1" dy="0.6" layer="1"/>
<smd name="4" x="1.3" y="0.95" dx="1.1" dy="0.6" layer="1"/>
<smd name="5" x="1.3" y="0" dx="1.1" dy="0.6" layer="1"/>
<smd name="6" x="1.3" y="-0.95" dx="1.1" dy="0.6" layer="1"/>
<wire x1="-0.8" y1="-1.5" x2="0.8" y2="-1.5" width="0.15" layer="21"/>
<wire x1="0.8" y1="-1.5" x2="0.8" y2="1.5" width="0.15" layer="21"/>
<wire x1="0.8" y1="1.5" x2="-0.8" y2="1.5" width="0.15" layer="21"/>
<wire x1="-0.8" y1="1.5" x2="-0.8" y2="-1.5" width="0.15" layer="21"/>
<circle x="-2.2" y="-0.95" radius="0.2" width="0.25" layer="21"/>
<text x="-2.2" y="2" size="1.016" layer="25">&gt;NAME</text>
<text x="-2.2" y="-3.2" size="1.016" layer="27">&gt;VALUE</text>
</package>
</packages>
<symbols>
<symbol name="MQS-8">
<description>Vehicle interface connector, 8 way</description>
<wire x1="-5.08" y1="12.7" x2="5.08" y2="12.7" width="0.254" layer="94"/>
<wire x1="5.08" y1="12.7" x2="5.08" y2="-12.7" width="0.254" layer="94"/>
<wire x1="5.08" y1="-12.7" x2="-5.08" y2="-12.7" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-12.7" x2="-5.08" y2="12.7" width="0.254" layer="94"/>
<pin name="1" x="-7.62" y="8.89" length="short" direction="pas"/>
<pin name="2" x="-7.62" y="6.35" length="short" direction="pas"/>
<pin name="3" x="-7.62" y="3.81" length="short" direction="pas"/>
<pin name="4" x="-7.62" y="1.27" length="short" direction="pas"/>
<pin name="5" x="-7.62" y="-1.27" length="short" direction="pas"/>
<pin name="6" x="-7.62" y="-3.81" length="short" direction="pas"/>
<pin name="7" x="-7.62" y="-6.35" length="short" direction="pas"/>
<pin name="8" x="-7.62" y="-8.89" length="short" direction="pas"/>
<text x="-5.08" y="13.462" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-15.24" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="MCP25625">
<description>CAN controller with integrated transceiver.
Left column = pads 1-14, right column = pads 15-28, matching the physical package.
Note: TXCAN/RXCAN must be wired externally to TXD/RXD - the controller and
transceiver dies are not connected internally.</description>
<wire x1="-15.24" y1="19.05" x2="15.24" y2="19.05" width="0.254" layer="94"/>
<wire x1="15.24" y1="19.05" x2="15.24" y2="-19.05" width="0.254" layer="94"/>
<wire x1="15.24" y1="-19.05" x2="-15.24" y2="-19.05" width="0.254" layer="94"/>
<wire x1="-15.24" y1="-19.05" x2="-15.24" y2="19.05" width="0.254" layer="94"/>
<pin name="VIO" x="-17.78" y="16.51" length="short" direction="pwr"/>
<pin name="NC1" x="-17.78" y="13.97" length="short" direction="nc"/>
<pin name="CANL" x="-17.78" y="11.43" length="short" direction="io"/>
<pin name="CANH" x="-17.78" y="8.89" length="short" direction="io"/>
<pin name="STBY" x="-17.78" y="6.35" length="short" direction="in"/>
<pin name="TX1RTS" x="-17.78" y="3.81" length="short" direction="in"/>
<pin name="TX2RTS" x="-17.78" y="1.27" length="short" direction="in"/>
<pin name="OSC2" x="-17.78" y="-1.27" length="short" direction="out"/>
<pin name="OSC1" x="-17.78" y="-3.81" length="short" direction="in"/>
<pin name="GND" x="-17.78" y="-6.35" length="short" direction="pwr"/>
<pin name="RX1BF" x="-17.78" y="-8.89" length="short" direction="out"/>
<pin name="RX0BF" x="-17.78" y="-11.43" length="short" direction="out"/>
<pin name="INT" x="-17.78" y="-13.97" length="short" direction="out"/>
<pin name="SCK" x="-17.78" y="-16.51" length="short" direction="in"/>
<pin name="SI" x="17.78" y="-16.51" length="short" direction="in" rot="R180"/>
<pin name="SO" x="17.78" y="-13.97" length="short" direction="out" rot="R180"/>
<pin name="CS" x="17.78" y="-11.43" length="short" direction="in" rot="R180"/>
<pin name="RESET" x="17.78" y="-8.89" length="short" direction="in" rot="R180"/>
<pin name="VDD" x="17.78" y="-6.35" length="short" direction="pwr" rot="R180"/>
<pin name="TXCAN" x="17.78" y="-3.81" length="short" direction="out" rot="R180"/>
<pin name="RXCAN" x="17.78" y="-1.27" length="short" direction="in" rot="R180"/>
<pin name="CLKOUT" x="17.78" y="1.27" length="short" direction="out" rot="R180"/>
<pin name="TX0RTS" x="17.78" y="3.81" length="short" direction="in" rot="R180"/>
<pin name="TXD" x="17.78" y="6.35" length="short" direction="in" rot="R180"/>
<pin name="NC2" x="17.78" y="8.89" length="short" direction="nc" rot="R180"/>
<pin name="VSS" x="17.78" y="11.43" length="short" direction="pwr" rot="R180"/>
<pin name="VDDA" x="17.78" y="13.97" length="short" direction="pwr" rot="R180"/>
<pin name="RXD" x="17.78" y="16.51" length="short" direction="out" rot="R180"/>
<text x="-15.24" y="19.812" size="1.778" layer="95">&gt;NAME</text>
<text x="-15.24" y="-21.59" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="RPI-PICO2">
<description>Raspberry Pi Pico 2 (RP2350). Left column = pins 1-20, right column = pins 21-40,
matching the physical module. 3V3OUT external load should stay under 300mA.</description>
<wire x1="-20.32" y1="26.67" x2="20.32" y2="26.67" width="0.254" layer="94"/>
<wire x1="20.32" y1="26.67" x2="20.32" y2="-26.67" width="0.254" layer="94"/>
<wire x1="20.32" y1="-26.67" x2="-20.32" y2="-26.67" width="0.254" layer="94"/>
<wire x1="-20.32" y1="-26.67" x2="-20.32" y2="26.67" width="0.254" layer="94"/>
<pin name="GP0" x="-22.86" y="24.13" length="short" direction="io"/>
<pin name="GP1" x="-22.86" y="21.59" length="short" direction="io"/>
<pin name="GND1" x="-22.86" y="19.05" length="short" direction="pwr"/>
<pin name="GP2" x="-22.86" y="16.51" length="short" direction="io"/>
<pin name="GP3" x="-22.86" y="13.97" length="short" direction="io"/>
<pin name="GP4" x="-22.86" y="11.43" length="short" direction="io"/>
<pin name="GP5" x="-22.86" y="8.89" length="short" direction="io"/>
<pin name="GND2" x="-22.86" y="6.35" length="short" direction="pwr"/>
<pin name="GP6" x="-22.86" y="3.81" length="short" direction="io"/>
<pin name="GP7" x="-22.86" y="1.27" length="short" direction="io"/>
<pin name="GP8" x="-22.86" y="-1.27" length="short" direction="io"/>
<pin name="GP9" x="-22.86" y="-3.81" length="short" direction="io"/>
<pin name="GND3" x="-22.86" y="-6.35" length="short" direction="pwr"/>
<pin name="GP10" x="-22.86" y="-8.89" length="short" direction="io"/>
<pin name="GP11" x="-22.86" y="-11.43" length="short" direction="io"/>
<pin name="GP12" x="-22.86" y="-13.97" length="short" direction="io"/>
<pin name="GP13" x="-22.86" y="-16.51" length="short" direction="io"/>
<pin name="GND4" x="-22.86" y="-19.05" length="short" direction="pwr"/>
<pin name="GP14" x="-22.86" y="-21.59" length="short" direction="io"/>
<pin name="GP15" x="-22.86" y="-24.13" length="short" direction="io"/>
<pin name="GP16" x="22.86" y="-24.13" length="short" direction="io" rot="R180"/>
<pin name="GP17" x="22.86" y="-21.59" length="short" direction="io" rot="R180"/>
<pin name="GND5" x="22.86" y="-19.05" length="short" direction="pwr" rot="R180"/>
<pin name="GP18" x="22.86" y="-16.51" length="short" direction="io" rot="R180"/>
<pin name="GP19" x="22.86" y="-13.97" length="short" direction="io" rot="R180"/>
<pin name="GP20" x="22.86" y="-11.43" length="short" direction="io" rot="R180"/>
<pin name="GP21" x="22.86" y="-8.89" length="short" direction="io" rot="R180"/>
<pin name="GND6" x="22.86" y="-6.35" length="short" direction="pwr" rot="R180"/>
<pin name="GP22" x="22.86" y="-3.81" length="short" direction="io" rot="R180"/>
<pin name="RUN" x="22.86" y="-1.27" length="short" direction="in" rot="R180"/>
<pin name="GP26_ADC0" x="22.86" y="1.27" length="short" direction="io" rot="R180"/>
<pin name="GP27_ADC1" x="22.86" y="3.81" length="short" direction="io" rot="R180"/>
<pin name="AGND" x="22.86" y="6.35" length="short" direction="pwr" rot="R180"/>
<pin name="GP28_ADC2" x="22.86" y="8.89" length="short" direction="io" rot="R180"/>
<pin name="ADC_VREF" x="22.86" y="11.43" length="short" direction="pwr" rot="R180"/>
<pin name="3V3OUT" x="22.86" y="13.97" length="short" direction="pwr" rot="R180"/>
<pin name="3V3EN" x="22.86" y="16.51" length="short" direction="in" rot="R180"/>
<pin name="GND7" x="22.86" y="19.05" length="short" direction="pwr" rot="R180"/>
<pin name="VSYS" x="22.86" y="21.59" length="short" direction="pwr" rot="R180"/>
<pin name="VBUS" x="22.86" y="24.13" length="short" direction="pwr" rot="R180"/>
<text x="-20.32" y="27.432" size="1.778" layer="95">&gt;NAME</text>
<text x="-20.32" y="-29.21" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="MICROSD-DM3AT">
<description>microSD socket. Left = card contacts 1-8, right = auxiliary pads
(shield legs and the card-detect switch), named by physical position.</description>
<wire x1="-15.24" y1="19.05" x2="15.24" y2="19.05" width="0.254" layer="94"/>
<wire x1="15.24" y1="19.05" x2="15.24" y2="-19.05" width="0.254" layer="94"/>
<wire x1="15.24" y1="-19.05" x2="-15.24" y2="-19.05" width="0.254" layer="94"/>
<wire x1="-15.24" y1="-19.05" x2="-15.24" y2="19.05" width="0.254" layer="94"/>
<pin name="DAT2" x="-17.78" y="16.51" length="short" direction="io"/>
<pin name="CD_DAT3" x="-17.78" y="13.97" length="short" direction="io"/>
<pin name="CMD" x="-17.78" y="11.43" length="short" direction="io"/>
<pin name="VDD" x="-17.78" y="8.89" length="short" direction="pwr"/>
<pin name="CLK" x="-17.78" y="6.35" length="short" direction="io"/>
<pin name="VSS" x="-17.78" y="3.81" length="short" direction="pwr"/>
<pin name="DAT0" x="-17.78" y="1.27" length="short" direction="io"/>
<pin name="DAT1" x="-17.78" y="-1.27" length="short" direction="io"/>
<pin name="TR" x="17.78" y="-16.51" length="short" direction="pas" rot="R180"/>
<pin name="TL" x="17.78" y="-13.97" length="short" direction="pas" rot="R180"/>
<pin name="L1" x="17.78" y="-11.43" length="short" direction="pas" rot="R180"/>
<pin name="L2" x="17.78" y="-8.89" length="short" direction="pas" rot="R180"/>
<pin name="L3" x="17.78" y="-6.35" length="short" direction="pas" rot="R180"/>
<pin name="BR" x="17.78" y="-3.81" length="short" direction="pas" rot="R180"/>
<text x="-15.24" y="19.812" size="1.778" layer="95">&gt;NAME</text>
<text x="-15.24" y="-21.59" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="GPS-5">
<description>GT-502MGG-N GNSS module, bare wire pads</description>
<wire x1="-5.08" y1="7.62" x2="5.08" y2="7.62" width="0.254" layer="94"/>
<wire x1="5.08" y1="7.62" x2="5.08" y2="-7.62" width="0.254" layer="94"/>
<wire x1="5.08" y1="-7.62" x2="-5.08" y2="-7.62" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-7.62" x2="-5.08" y2="7.62" width="0.254" layer="94"/>
<pin name="VCC" x="-7.62" y="5.08" length="short" direction="pwr"/>
<pin name="GND" x="-7.62" y="2.54" length="short" direction="pwr"/>
<pin name="TXD" x="-7.62" y="0" length="short" direction="out"/>
<pin name="RXD" x="-7.62" y="-2.54" length="short" direction="in"/>
<pin name="PPS" x="-7.62" y="-5.08" length="short" direction="out"/>
<text x="-5.08" y="8.382" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-10.16" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="R">
<description>Resistor</description>
<pin name="1" x="-5.08" y="0" length="short" direction="pas"/>
<pin name="2" x="5.08" y="0" length="short" direction="pas" rot="R180"/>
<wire x1="-2.54" y1="-0.889" x2="2.54" y2="-0.889" width="0.254" layer="94"/>
<wire x1="2.54" y1="-0.889" x2="2.54" y2="0.889" width="0.254" layer="94"/>
<wire x1="2.54" y1="0.889" x2="-2.54" y2="0.889" width="0.254" layer="94"/>
<wire x1="-2.54" y1="0.889" x2="-2.54" y2="-0.889" width="0.254" layer="94"/>
<text x="-2.54" y="1.651" size="1.778" layer="95">&gt;NAME</text>
<text x="-2.54" y="-3.302" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="C">
<description>Non-polarised capacitor</description>
<pin name="1" x="-5.08" y="0" length="short" direction="pas"/>
<pin name="2" x="5.08" y="0" length="short" direction="pas" rot="R180"/>
<wire x1="-2.54" y1="0" x2="-0.762" y2="0" width="0.254" layer="94"/>
<wire x1="0.762" y1="0" x2="2.54" y2="0" width="0.254" layer="94"/>
<wire x1="-0.762" y1="1.524" x2="-0.762" y2="-1.524" width="0.4" layer="94"/>
<wire x1="0.762" y1="1.524" x2="0.762" y2="-1.524" width="0.4" layer="94"/>
<text x="-2.54" y="2.286" size="1.778" layer="95">&gt;NAME</text>
<text x="-2.54" y="-3.81" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="D">
<description>Diode / Schottky diode</description>
<pin name="A" x="-5.08" y="0" length="short" direction="pas"/>
<pin name="K" x="5.08" y="0" length="short" direction="pas" rot="R180"/>
<wire x1="-2.54" y1="0" x2="-1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="2.54" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="-1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="-1.27" width="0.254" layer="94"/>
<text x="-2.54" y="2.286" size="1.778" layer="95">&gt;NAME</text>
<text x="-2.54" y="-3.81" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="ZD">
<description>Zener / TVS diode</description>
<pin name="A" x="-5.08" y="0" length="short" direction="pas"/>
<pin name="K" x="5.08" y="0" length="short" direction="pas" rot="R180"/>
<wire x1="-2.54" y1="0" x2="-1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="2.54" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="-1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="2.032" y2="1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="-1.27" x2="0.508" y2="-1.27" width="0.254" layer="94"/>
<text x="-2.54" y="2.286" size="1.778" layer="95">&gt;NAME</text>
<text x="-2.54" y="-3.81" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="LED">
<description>LED</description>
<pin name="K" x="-5.08" y="0" length="short" direction="pas"/>
<pin name="A" x="5.08" y="0" length="short" direction="pas" rot="R180"/>
<wire x1="-2.54" y1="0" x2="-1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="2.54" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="-1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="-1.27" x2="-1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-0.5" y1="1.8" x2="0.6" y2="2.9" width="0.2" layer="94"/>
<wire x1="0.6" y1="2.9" x2="0.1" y2="2.6" width="0.2" layer="94"/>
<wire x1="0.6" y1="2.9" x2="0.35" y2="2.35" width="0.2" layer="94"/>
<wire x1="0.8" y1="1.8" x2="1.9" y2="2.9" width="0.2" layer="94"/>
<wire x1="1.9" y1="2.9" x2="1.4" y2="2.6" width="0.2" layer="94"/>
<wire x1="1.9" y1="2.9" x2="1.65" y2="2.35" width="0.2" layer="94"/>
<text x="-2.54" y="3.556" size="1.778" layer="95">&gt;NAME</text>
<text x="-2.54" y="-3.81" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="FUSE">
<description>Resettable fuse (PTC)</description>
<pin name="1" x="-5.08" y="0" length="short" direction="pas"/>
<pin name="2" x="5.08" y="0" length="short" direction="pas" rot="R180"/>
<wire x1="-2.54" y1="-0.889" x2="2.54" y2="-0.889" width="0.254" layer="94"/>
<wire x1="2.54" y1="-0.889" x2="2.54" y2="0.889" width="0.254" layer="94"/>
<wire x1="2.54" y1="0.889" x2="-2.54" y2="0.889" width="0.254" layer="94"/>
<wire x1="-2.54" y1="0.889" x2="-2.54" y2="-0.889" width="0.254" layer="94"/>
<wire x1="-2.54" y1="0" x2="2.54" y2="0" width="0.254" layer="94"/>
<text x="-2.54" y="1.651" size="1.778" layer="95">&gt;NAME</text>
<text x="-2.54" y="-3.302" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="XTAL">
<description>Crystal</description>
<pin name="1" x="-5.08" y="0" length="short" direction="pas"/>
<pin name="2" x="5.08" y="0" length="short" direction="pas" rot="R180"/>
<wire x1="-2.54" y1="0" x2="-1.524" y2="0" width="0.254" layer="94"/>
<wire x1="1.524" y1="0" x2="2.54" y2="0" width="0.254" layer="94"/>
<wire x1="-1.524" y1="1.778" x2="-1.524" y2="-1.778" width="0.4" layer="94"/>
<wire x1="1.524" y1="1.778" x2="1.524" y2="-1.778" width="0.4" layer="94"/>
<wire x1="-0.762" y1="1.778" x2="0.762" y2="1.778" width="0.254" layer="94"/>
<wire x1="0.762" y1="1.778" x2="0.762" y2="-1.778" width="0.254" layer="94"/>
<wire x1="0.762" y1="-1.778" x2="-0.762" y2="-1.778" width="0.254" layer="94"/>
<wire x1="-0.762" y1="-1.778" x2="-0.762" y2="1.778" width="0.254" layer="94"/>
<text x="-2.54" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<text x="-2.54" y="-4.064" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="JP2">
<description>2-way solder jumper</description>
<pin name="1" x="-5.08" y="0" length="short" direction="pas"/>
<pin name="2" x="5.08" y="0" length="short" direction="pas" rot="R180"/>
<wire x1="-2.54" y1="0" x2="-1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="2.54" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="-1.27" width="0.254" layer="94"/>
<text x="-2.54" y="2.286" size="1.778" layer="95">&gt;NAME</text>
<text x="-2.54" y="-3.81" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="NPN">
<description>NPN transistor</description>
<pin name="B" x="-7.62" y="0" length="short" direction="in"/>
<pin name="C" x="2.54" y="7.62" length="short" direction="pas" rot="R270"/>
<pin name="E" x="2.54" y="-7.62" length="short" direction="pas" rot="R90"/>
<wire x1="-5.08" y1="0" x2="-2.54" y2="0" width="0.254" layer="94"/>
<wire x1="-2.54" y1="2.54" x2="-2.54" y2="-2.54" width="0.4" layer="94"/>
<wire x1="-2.54" y1="1.27" x2="2.54" y2="5.08" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-1.27" x2="2.54" y2="-5.08" width="0.254" layer="94"/>
<wire x1="1.27" y1="-3.556" x2="2.286" y2="-4.572" width="0.6" layer="94"/>
<text x="4" y="5.08" size="1.778" layer="95">&gt;NAME</text>
<text x="4" y="-6.35" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="CAN-TVS">
<description>Dual TVS array for CAN, common ground (PESD1CAN)</description>
<pin name="1" x="-7.62" y="2.54" length="short" direction="pas"/>
<pin name="2" x="0" y="-7.62" length="short" direction="pwr" rot="R90"/>
<pin name="3" x="-7.62" y="-2.54" length="short" direction="pas"/>
<wire x1="-5.08" y1="2.54" x2="-1.27" y2="2.54" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-2.54" x2="-1.27" y2="-2.54" width="0.254" layer="94"/>
<wire x1="-1.27" y1="3.81" x2="-1.27" y2="1.27" width="0.4" layer="94"/>
<wire x1="1.27" y1="3.81" x2="1.27" y2="1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="3.81" x2="-1.27" y2="2.54" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="-1.27" y2="2.54" width="0.254" layer="94"/>
<wire x1="-1.27" y1="-1.27" x2="-1.27" y2="-3.81" width="0.4" layer="94"/>
<wire x1="1.27" y1="-1.27" x2="1.27" y2="-3.81" width="0.254" layer="94"/>
<wire x1="1.27" y1="-1.27" x2="-1.27" y2="-2.54" width="0.254" layer="94"/>
<wire x1="1.27" y1="-3.81" x2="-1.27" y2="-2.54" width="0.254" layer="94"/>
<wire x1="1.27" y1="2.54" x2="2.54" y2="2.54" width="0.254" layer="94"/>
<wire x1="1.27" y1="-2.54" x2="2.54" y2="-2.54" width="0.254" layer="94"/>
<wire x1="2.54" y1="2.54" x2="2.54" y2="-2.54" width="0.254" layer="94"/>
<wire x1="2.54" y1="0" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="-5.08" width="0.254" layer="94"/>
<text x="-5.08" y="5.08" size="1.778" layer="95">&gt;NAME</text>
<text x="-5.08" y="-9.5" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="REG3">
<description>Three-terminal regulator / switching regulator module</description>
<wire x1="-10.16" y1="5.08" x2="10.16" y2="5.08" width="0.254" layer="94"/>
<wire x1="10.16" y1="5.08" x2="10.16" y2="-5.08" width="0.254" layer="94"/>
<wire x1="10.16" y1="-5.08" x2="-10.16" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-10.16" y1="-5.08" x2="-10.16" y2="5.08" width="0.254" layer="94"/>
<pin name="VIN" x="-12.7" y="2.54" length="short" direction="pwr"/>
<pin name="VOUT" x="12.7" y="2.54" length="short" direction="pwr" rot="R180"/>
<pin name="GND" x="0" y="-7.62" length="short" direction="pwr" rot="R90"/>
<text x="-10.16" y="5.842" size="1.778" layer="95">&gt;NAME</text>
<text x="-10.16" y="-8.89" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
<symbol name="PMOS">
<description>P-channel enhancement MOSFET</description>
<pin name="G" x="-7.62" y="0" length="short" direction="in"/>
<pin name="S" x="0" y="7.62" length="short" direction="pas" rot="R270"/>
<pin name="D" x="0" y="-7.62" length="short" direction="pas" rot="R90"/>
<wire x1="-5.08" y1="0" x2="-2.54" y2="0" width="0.254" layer="94"/>
<wire x1="-2.54" y1="2.54" x2="-2.54" y2="-2.54" width="0.4" layer="94"/>
<wire x1="-1.27" y1="2.54" x2="-1.27" y2="1.27" width="0.4" layer="94"/>
<wire x1="-1.27" y1="0.635" x2="-1.27" y2="-0.635" width="0.4" layer="94"/>
<wire x1="-1.27" y1="-1.27" x2="-1.27" y2="-2.54" width="0.4" layer="94"/>
<wire x1="-1.27" y1="1.905" x2="0" y2="1.905" width="0.254" layer="94"/>
<wire x1="0" y1="1.905" x2="0" y2="5.08" width="0.254" layer="94"/>
<wire x1="-1.27" y1="-1.905" x2="0" y2="-1.905" width="0.254" layer="94"/>
<wire x1="0" y1="-1.905" x2="0" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-1.27" y1="0" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="1.905" width="0.254" layer="94"/>
<wire x1="-0.508" y1="0" x2="0.254" y2="0.508" width="0.4" layer="94"/>
<wire x1="-0.508" y1="0" x2="0.254" y2="-0.508" width="0.4" layer="94"/>
<text x="2.54" y="4.064" size="1.778" layer="95">&gt;NAME</text>
<text x="2.54" y="-5.588" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="MQS-8-967658" prefix="J">
<description>TE 1-967658-1 vehicle interface connector (MQS .63, 8 way, right angle).
MotoRecoPico assignment: 3=IG 12V, 4=GND, 5=CAN Low, 6=CAN High, 8=12V permanent.
Pins 1, 2, 7 unused.</description>
<gates>
<gate name="G$1" symbol="MQS-8" x="0" y="0"/>
</gates>
<devices>
<device name="" package="MQS-8-RA-967658">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
<connect gate="G$1" pin="3" pad="3"/>
<connect gate="G$1" pin="4" pad="4"/>
<connect gate="G$1" pin="5" pad="5"/>
<connect gate="G$1" pin="6" pad="6"/>
<connect gate="G$1" pin="7" pad="7"/>
<connect gate="G$1" pin="8" pad="8"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="MCP25625" prefix="U">
<description>Microchip MCP25625 CAN 2.0B controller with integrated transceiver, SSOP-28.
VDDA must be 5V. VDD and VIO tie to 3.3V for direct connection to a 3.3V MCU
with no level shifters.
TXD must be wired to TXCAN and RXD to RXCAN on the PCB.</description>
<gates>
<gate name="G$1" symbol="MCP25625" x="0" y="0"/>
</gates>
<devices>
<device name="T-E/SS" package="SSOP28">
<connects>
<connect gate="G$1" pin="VIO" pad="1"/>
<connect gate="G$1" pin="NC1" pad="2"/>
<connect gate="G$1" pin="CANL" pad="3"/>
<connect gate="G$1" pin="CANH" pad="4"/>
<connect gate="G$1" pin="STBY" pad="5"/>
<connect gate="G$1" pin="TX1RTS" pad="6"/>
<connect gate="G$1" pin="TX2RTS" pad="7"/>
<connect gate="G$1" pin="OSC2" pad="8"/>
<connect gate="G$1" pin="OSC1" pad="9"/>
<connect gate="G$1" pin="GND" pad="10"/>
<connect gate="G$1" pin="RX1BF" pad="11"/>
<connect gate="G$1" pin="RX0BF" pad="12"/>
<connect gate="G$1" pin="INT" pad="13"/>
<connect gate="G$1" pin="SCK" pad="14"/>
<connect gate="G$1" pin="SI" pad="15"/>
<connect gate="G$1" pin="SO" pad="16"/>
<connect gate="G$1" pin="CS" pad="17"/>
<connect gate="G$1" pin="RESET" pad="18"/>
<connect gate="G$1" pin="VDD" pad="19"/>
<connect gate="G$1" pin="TXCAN" pad="20"/>
<connect gate="G$1" pin="RXCAN" pad="21"/>
<connect gate="G$1" pin="CLKOUT" pad="22"/>
<connect gate="G$1" pin="TX0RTS" pad="23"/>
<connect gate="G$1" pin="TXD" pad="24"/>
<connect gate="G$1" pin="NC2" pad="25"/>
<connect gate="G$1" pin="VSS" pad="26"/>
<connect gate="G$1" pin="VDDA" pad="27"/>
<connect gate="G$1" pin="RXD" pad="28"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="RPI-PICO2" prefix="U">
<description>Raspberry Pi Pico 2 (RP2350) module.
MotoRecoPico usage: SPI1 (GP10-13) to MCP25625, SPI0 (GP16-19) to MicroSD,
UART0 (GP0/GP1) to GPS, GP2 = 1PPS, GP3 = IG sense, GP22 = KEEP_ALIVE,
GP28 = battery voltage sense. VSYS fed from the 5V rail via a Schottky diode.</description>
<gates>
<gate name="G$1" symbol="RPI-PICO2" x="0" y="0"/>
</gates>
<devices>
<device name="" package="RPI-PICO2-THT">
<connects>
<connect gate="G$1" pin="GP0" pad="1"/>
<connect gate="G$1" pin="GP1" pad="2"/>
<connect gate="G$1" pin="GND1" pad="3"/>
<connect gate="G$1" pin="GP2" pad="4"/>
<connect gate="G$1" pin="GP3" pad="5"/>
<connect gate="G$1" pin="GP4" pad="6"/>
<connect gate="G$1" pin="GP5" pad="7"/>
<connect gate="G$1" pin="GND2" pad="8"/>
<connect gate="G$1" pin="GP6" pad="9"/>
<connect gate="G$1" pin="GP7" pad="10"/>
<connect gate="G$1" pin="GP8" pad="11"/>
<connect gate="G$1" pin="GP9" pad="12"/>
<connect gate="G$1" pin="GND3" pad="13"/>
<connect gate="G$1" pin="GP10" pad="14"/>
<connect gate="G$1" pin="GP11" pad="15"/>
<connect gate="G$1" pin="GP12" pad="16"/>
<connect gate="G$1" pin="GP13" pad="17"/>
<connect gate="G$1" pin="GND4" pad="18"/>
<connect gate="G$1" pin="GP14" pad="19"/>
<connect gate="G$1" pin="GP15" pad="20"/>
<connect gate="G$1" pin="GP16" pad="21"/>
<connect gate="G$1" pin="GP17" pad="22"/>
<connect gate="G$1" pin="GND5" pad="23"/>
<connect gate="G$1" pin="GP18" pad="24"/>
<connect gate="G$1" pin="GP19" pad="25"/>
<connect gate="G$1" pin="GP20" pad="26"/>
<connect gate="G$1" pin="GP21" pad="27"/>
<connect gate="G$1" pin="GND6" pad="28"/>
<connect gate="G$1" pin="GP22" pad="29"/>
<connect gate="G$1" pin="RUN" pad="30"/>
<connect gate="G$1" pin="GP26_ADC0" pad="31"/>
<connect gate="G$1" pin="GP27_ADC1" pad="32"/>
<connect gate="G$1" pin="AGND" pad="33"/>
<connect gate="G$1" pin="GP28_ADC2" pad="34"/>
<connect gate="G$1" pin="ADC_VREF" pad="35"/>
<connect gate="G$1" pin="3V3OUT" pad="36"/>
<connect gate="G$1" pin="3V3EN" pad="37"/>
<connect gate="G$1" pin="GND7" pad="38"/>
<connect gate="G$1" pin="VSYS" pad="39"/>
<connect gate="G$1" pin="VBUS" pad="40"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="MICROSD-DM3AT-SF-PEJM5" prefix="J">
<description>Hirose DM3AT-SF-PEJM5 microSD socket, push-push, card detect, 0.5A, 10000 cycles.
Hand soldering is supported by Hirose (iron 350C for 3s).
SPI mode wiring: CD_DAT3=CS, CMD=MOSI, CLK=SCK, DAT0=MISO, VDD=3.3V, VSS=GND.
DAT1 and DAT2 are unused in SPI mode but should be pulled up per the SD spec.
Card detect switch is normally open and closes when a card is inserted.
The six auxiliary pads (TL, TR, L1, L2, L3, BR) are the stainless cover ground legs
plus the two card-detect switch terminals. From the Hirose drawing the switch is
most likely TR (drawing calls out card-detect(B) at the right end of the contact row)
and BR (card-detect(A) on the lower right flank); the remaining four are ground legs.
VERIFY WITH A CONTINUITY CHECK before committing the schematic: the two switch pads
are the pair that goes from open to shorted when a card is inserted. Everything else
goes to GND.</description>
<gates>
<gate name="G$1" symbol="MICROSD-DM3AT" x="0" y="0"/>
</gates>
<devices>
<device name="" package="MICROSD-DM3AT">
<connects>
<connect gate="G$1" pin="DAT2" pad="1"/>
<connect gate="G$1" pin="CD_DAT3" pad="2"/>
<connect gate="G$1" pin="CMD" pad="3"/>
<connect gate="G$1" pin="VDD" pad="4"/>
<connect gate="G$1" pin="CLK" pad="5"/>
<connect gate="G$1" pin="VSS" pad="6"/>
<connect gate="G$1" pin="DAT0" pad="7"/>
<connect gate="G$1" pin="DAT1" pad="8"/>
<connect gate="G$1" pin="TR" pad="TR"/>
<connect gate="G$1" pin="TL" pad="TL"/>
<connect gate="G$1" pin="L1" pad="L1"/>
<connect gate="G$1" pin="L2" pad="L2"/>
<connect gate="G$1" pin="L3" pad="L3"/>
<connect gate="G$1" pin="BR" pad="BR"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="R" prefix="R">
<description>Chip resistor, 0805.</description>
<gates>
<gate name="G$1" symbol="R" x="0" y="0"/>
</gates>
<devices>
<device name="" package="C1005">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="C" prefix="C">
<description>Chip capacitor. Default device is 1005 metric (0402).
Use the -1608 device for the 10uF bulk capacitors, which are not
practically obtainable in 1005.</description>
<gates>
<gate name="G$1" symbol="C" x="0" y="0"/>
</gates>
<devices>
<device name="" package="C1005">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="-1608" package="C1608">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="PTC-FUSE" prefix="F">
<description>Resettable PTC fuse, 1206. F1 = 1A hold.
Rated for the 1A hold value because PTC hold current derates heavily with
temperature - a 0.5A part holds only about 0.25A at 85C, too close to the
250mA peak load, and would nuisance-trip.</description>
<gates>
<gate name="G$1" symbol="FUSE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="C1206">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CRYSTAL" prefix="Y">
<description>HC-49/S through-hole crystal. Y1 = 16MHz for the MCP25625.
16MHz divides exactly for 500k, 250k and 125k bit rates.</description>
<gates>
<gate name="G$1" symbol="XTAL" x="0" y="0"/>
</gates>
<devices>
<device name="" package="HC49S">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="SOLDER-JUMPER" prefix="JP">
<description>Two-pad solder jumper. JP1 breaks the CAN split-termination midpoint.
DEFAULT STATE IS CLOSED - open it only to rule out over-termination on a bus
that is already terminated at both ends.</description>
<gates>
<gate name="G$1" symbol="JP2" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SJ2">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="LED-SIDEVIEW" prefix="D">
<description>Side-view chip LED, 2.8 x 1.2 x 0.8mm. One footprint fits both fitted parts:
D10 = Kodenshi LP812-010(T) green (Akizuki 116854), VF 2.8-3.0V
D11 = Sharp GM4ZR83200AE red (Akizuki 105328), VF 2.1V typ / 2.6V max
Driven low-side from the 5V rail through 1k, so the GPIO sinks the current and
LOW LIGHTS THE LED. The green part cannot be driven from 3.3V - its VF leaves
only 0.3V of headroom there.
Keep the current near 2-3mA: the red part derates 0.27mA/C and allows only
about 3.8mA at 85C. Use blinking rather than more current for visibility.</description>
<gates>
<gate name="G$1" symbol="LED" x="0" y="0"/>
</gates>
<devices>
<device name="" package="LED-SIDE-2812">
<connects>
<connect gate="G$1" pin="K" pad="1"/>
<connect gate="G$1" pin="A" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="PESD1CAN" prefix="D">
<description>PESD1CAN dual TVS array for CAN, SOT-23. Pin 2 is ground; pins 1 and 3 go to
CANH and CANL. The device is symmetric so the bus pins may be swapped.</description>
<gates>
<gate name="G$1" symbol="CAN-TVS" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOT23">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
<connect gate="G$1" pin="3" pad="3"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="MMBT3904" prefix="Q">
<description>MMBT3904 NPN, SOT-23. Q1 pulls the LM2596 ON/OFF pin low to enable the supply.</description>
<gates>
<gate name="G$1" symbol="NPN" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOT23">
<connects>
<connect gate="G$1" pin="B" pad="1"/>
<connect gate="G$1" pin="E" pad="2"/>
<connect gate="G$1" pin="C" pad="3"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="M78AR05-0.5" prefix="U">
<description>MinMax M78AR05-0.5 switching regulator module, 6.5-32V in, 5V 500mA out.
Drop-in for an LM7805 but switching, so no heatsink is needed. Inductor and
capacitors are inside the module - no external switching components.
Short-circuit protection, thermal shutdown, 1A output current limit.
NO ENABLE PIN, which is why Q2 switches its input instead.
Needs 22uF/50V on the input to be rated for 32V (datasheet note 7).
No-load input current is 5mA, so it must never sit on permanent 12V unswitched.
Recommended input fuse for the 5V model is 1A slow-blow - that is F1.</description>
<gates>
<gate name="G$1" symbol="REG3" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SIP3-M78AR05">
<connects>
<connect gate="G$1" pin="VIN" pad="1"/>
<connect gate="G$1" pin="GND" pad="2"/>
<connect gate="G$1" pin="VOUT" pad="3"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="ZXMP6A17" prefix="Q">
<description>Diodes ZXMP6A17E6Q P-channel MOSFET, SOT-26. -60V, -3.0A, Vgs +/-20V,
Rds(on) 125mOhm at Vgs=-10V and 190mOhm at -4.5V. AEC-Q101 qualified.
High-side switch for the 12V feed, turned on by the vehicle IG signal.
Gate is driven by a 100k/100k divider so Vgs is always half the supply:
6V at 12V in, 7.2V at 14.4V, 14.6V at the 29.2V TVS clamp - always inside
the +/-20V rating and always fully enhanced, with no gate zener needed.
Drain is four pads (thermal path) tied to one terminal.</description>
<gates>
<gate name="G$1" symbol="PMOS" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOT26">
<connects>
<connect gate="G$1" pin="G" pad="1"/>
<connect gate="G$1" pin="S" pad="6"/>
<connect gate="G$1" pin="D" pad="2 3 4 5"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="GT-502MGG-N" prefix="J">
<description>GT-502MGG-N GNSS receiver, external module with a 1.5m bare-wire cable.
Wires solder to surface pads on one face only. Supply is the Pico 3V3(OUT):
50mA on top of the SD and CAN loads still leaves margin under the 300mA
recommended limit. 3.3V also keeps its UART levels safe for the Pico.</description>
<gates>
<gate name="G$1" symbol="GPS-5" x="0" y="0"/>
</gates>
<devices>
<device name="" package="GPS-5P-SMD">
<connects>
<connect gate="G$1" pin="VCC" pad="1"/>
<connect gate="G$1" pin="GND" pad="2"/>
<connect gate="G$1" pin="TXD" pad="3"/>
<connect gate="G$1" pin="RXD" pad="4"/>
<connect gate="G$1" pin="PPS" pad="5"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="SCHOTTKY-1A" prefix="D">
<description>Schottky diode, 1A / 40V (SS14, B140 or similar), SOD-123.
D1 = reverse-polarity protection in series with the 12V feed.
D3 = VSYS feed, diode-ORed with the Pico USB input.
Sized for the real 200mA load - the previous 3A SMA part was 15x oversized.</description>
<gates>
<gate name="G$1" symbol="D" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOD123">
<connects>
<connect gate="G$1" pin="A" pad="2"/>
<connect gate="G$1" pin="K" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="TVS-SMA" prefix="D">
<description>Unidirectional TVS in SMA. D2 = SMAJ16A, 400W, on the 12V input.
Standoff 16V clears the 14.4V charging rail; clamping is 26.0V max.
26V matters: with no external input capacitor U1 is rated 28V, not 32V
(the 32V figure in the M78AR05 datasheet requires a 22uF/50V input cap).
SMAJ18A would clamp at 29.2V and overshoot that limit - do not substitute it.
TVS size is surge energy, so this is the one part not shrunk casually:
SMA is 400W where SMB would be 600W.</description>
<gates>
<gate name="G$1" symbol="ZD" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SMA">
<connects>
<connect gate="G$1" pin="A" pad="2"/>
<connect gate="G$1" pin="K" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="ZENER-3V3" prefix="D">
<description>3.3V zener, SOD-123. D4 clamps the divided IG sense line to protect the ADC.</description>
<gates>
<gate name="G$1" symbol="ZD" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOD123">
<connects>
<connect gate="G$1" pin="A" pad="2"/>
<connect gate="G$1" pin="K" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes/>
<variantdefs/>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="C10" library="MotoRecoPico" deviceset="C" device="" value="100nF"/>
<part name="C11" library="MotoRecoPico" deviceset="C" device="-1608" value="10uF"/>
<part name="C12" library="MotoRecoPico" deviceset="C" device="" value="100nF"/>
<part name="C2" library="MotoRecoPico" deviceset="C" device="" value="100nF/50V"/>
<part name="C3" library="MotoRecoPico" deviceset="C" device="" value="100nF"/>
<part name="C4" library="MotoRecoPico" deviceset="C" device="" value="100nF"/>
<part name="C5" library="MotoRecoPico" deviceset="C" device="" value="100nF"/>
<part name="C6" library="MotoRecoPico" deviceset="C" device="-1608" value="10uF"/>
<part name="C7" library="MotoRecoPico" deviceset="C" device="" value="22pF"/>
<part name="C8" library="MotoRecoPico" deviceset="C" device="" value="22pF"/>
<part name="C9" library="MotoRecoPico" deviceset="C" device="" value="4.7nF"/>
<part name="D1" library="MotoRecoPico" deviceset="SCHOTTKY-1A" device="" value="SS14"/>
<part name="D2" library="MotoRecoPico" deviceset="TVS-SMA" device="" value="SMAJ16A"/>
<part name="D3" library="MotoRecoPico" deviceset="SCHOTTKY-1A" device="" value="SS14"/>
<part name="D4" library="MotoRecoPico" deviceset="ZENER-3V3" device="" value="3.3V"/>
<part name="D5" library="MotoRecoPico" deviceset="PESD1CAN" device="" value="PESD1CAN"/>
<part name="D6" library="MotoRecoPico" deviceset="LED-SIDEVIEW" device="" value="GREEN"/>
<part name="F1" library="MotoRecoPico" deviceset="PTC-FUSE" device="" value="1A hold"/>
<part name="J1" library="MotoRecoPico" deviceset="MQS-8-967658" device="" value="1-967658-1"/>
<part name="J2" library="MotoRecoPico" deviceset="MICROSD-DM3AT-SF-PEJM5" device="" value="DM3AT-SF-PEJM5"/>
<part name="J3" library="MotoRecoPico" deviceset="GT-502MGG-N" device="" value="GT-502MGG-N"/>
<part name="JP1" library="MotoRecoPico" deviceset="SOLDER-JUMPER" device="" value="CLOSED"/>
<part name="Q1" library="MotoRecoPico" deviceset="MMBT3904" device="" value="MMBT3904"/>
<part name="Q2" library="MotoRecoPico" deviceset="ZXMP6A17" device="" value="ZXMP6A17E6Q"/>
<part name="R1" library="MotoRecoPico" deviceset="R" device="" value="100k"/>
<part name="R10" library="MotoRecoPico" deviceset="R" device="" value="10k"/>
<part name="R11" library="MotoRecoPico" deviceset="R" device="" value="10k"/>
<part name="R12" library="MotoRecoPico" deviceset="R" device="" value="10k"/>
<part name="R13" library="MotoRecoPico" deviceset="R" device="" value="10k"/>
<part name="R14" library="MotoRecoPico" deviceset="R" device="" value="1k"/>
<part name="R2" library="MotoRecoPico" deviceset="R" device="" value="100k"/>
<part name="R3" library="MotoRecoPico" deviceset="R" device="" value="100k"/>
<part name="R4" library="MotoRecoPico" deviceset="R" device="" value="22k"/>
<part name="R5" library="MotoRecoPico" deviceset="R" device="" value="10k"/>
<part name="R6" library="MotoRecoPico" deviceset="R" device="" value="10k"/>
<part name="R7" library="MotoRecoPico" deviceset="R" device="" value="10k"/>
<part name="R8" library="MotoRecoPico" deviceset="R" device="" value="60R"/>
<part name="R9" library="MotoRecoPico" deviceset="R" device="" value="60R"/>
<part name="U1" library="MotoRecoPico" deviceset="M78AR05-0.5" device="" value="M78AR05-0.5"/>
<part name="U2" library="MotoRecoPico" deviceset="MCP25625" device="T-E/SS" value="MCP25625"/>
<part name="U3" library="MotoRecoPico" deviceset="RPI-PICO2" device="" value="Pico 2"/>
<part name="Y1" library="MotoRecoPico" deviceset="CRYSTAL" device="" value="16MHz"/>
</parts>
<sheets>
<sheet>
<plain>
<wire x1="18" y1="206" x2="192" y2="206" width="0.2" layer="94"/>
<wire x1="192" y1="206" x2="192" y2="268" width="0.2" layer="94"/>
<wire x1="192" y1="268" x2="18" y2="268" width="0.2" layer="94"/>
<wire x1="18" y1="268" x2="18" y2="206" width="0.2" layer="94"/>
<text x="20" y="263" size="2.54" layer="94">A  VEHICLE INTERFACE / PROTECTION / HIGH-SIDE SWITCH</text>
<wire x1="200" y1="206" x2="286" y2="206" width="0.2" layer="94"/>
<wire x1="286" y1="206" x2="286" y2="268" width="0.2" layer="94"/>
<wire x1="286" y1="268" x2="200" y2="268" width="0.2" layer="94"/>
<wire x1="200" y1="268" x2="200" y2="206" width="0.2" layer="94"/>
<text x="202" y="263" size="2.54" layer="94">B  5V SWITCHING REGULATOR</text>
<wire x1="18" y1="130" x2="252" y2="130" width="0.2" layer="94"/>
<wire x1="252" y1="130" x2="252" y2="188" width="0.2" layer="94"/>
<wire x1="252" y1="188" x2="18" y2="188" width="0.2" layer="94"/>
<wire x1="18" y1="188" x2="18" y2="130" width="0.2" layer="94"/>
<text x="20" y="183" size="2.54" layer="94">C  CAN  (MCP25625)</text>
<wire x1="260" y1="130" x2="356" y2="130" width="0.2" layer="94"/>
<wire x1="356" y1="130" x2="356" y2="268" width="0.2" layer="94"/>
<wire x1="356" y1="268" x2="260" y2="268" width="0.2" layer="94"/>
<wire x1="260" y1="268" x2="260" y2="130" width="0.2" layer="94"/>
<text x="262" y="263" size="2.54" layer="94">F  MCU  (Raspberry Pi Pico 2)</text>
<wire x1="18" y1="56" x2="192" y2="56" width="0.2" layer="94"/>
<wire x1="192" y1="56" x2="192" y2="114" width="0.2" layer="94"/>
<wire x1="192" y1="114" x2="18" y2="114" width="0.2" layer="94"/>
<wire x1="18" y1="114" x2="18" y2="56" width="0.2" layer="94"/>
<text x="20" y="109" size="2.54" layer="94">D  MICROSD</text>
<wire x1="200" y1="56" x2="356" y2="56" width="0.2" layer="94"/>
<wire x1="356" y1="56" x2="356" y2="114" width="0.2" layer="94"/>
<wire x1="356" y1="114" x2="200" y2="114" width="0.2" layer="94"/>
<wire x1="200" y1="114" x2="200" y2="56" width="0.2" layer="94"/>
<text x="202" y="109" size="2.54" layer="94">E  GPS / STATUS LED</text>
<text x="18" y="276" size="4.5" layer="94">MotoRecoPico - Raspberry Pi Pico 2 CAN logger</text>
<text x="18" y="271" size="2" layer="94">Every net is drawn as wire - no labels are load-bearing. Rails run as horizontal trunks with risers on the left. LEDs are low-side driven from V5: GPIO LOW lights the LED.</text>
</plain>
<instances>
<instance part="C10" gate="G$1" x="30.48" y="53.34"/>
<instance part="C11" gate="G$1" x="58.42" y="53.34"/>
<instance part="C12" gate="G$1" x="58.42" y="187.96"/>
<instance part="C2" gate="G$1" x="142.24" y="259.08"/>
<instance part="C3" gate="G$1" x="83.82" y="177.8"/>
<instance part="C4" gate="G$1" x="111.76" y="177.8"/>
<instance part="C5" gate="G$1" x="139.7" y="177.8"/>
<instance part="C6" gate="G$1" x="167.64" y="177.8"/>
<instance part="C7" gate="G$1" x="223.52" y="177.8"/>
<instance part="C8" gate="G$1" x="30.48" y="127"/>
<instance part="C9" gate="G$1" x="223.52" y="127"/>
<instance part="D1" gate="G$1" x="86.36" y="259.08"/>
<instance part="D2" gate="G$1" x="114.3" y="259.08"/>
<instance part="D3" gate="G$1" x="256.54" y="259.08"/>
<instance part="D4" gate="G$1" x="30.48" y="187.96"/>
<instance part="D5" gate="G$1" x="116.84" y="124.46"/>
<instance part="D6" gate="G$1" x="269.24" y="104.14"/>
<instance part="F1" gate="G$1" x="58.42" y="259.08"/>
<instance part="J1" gate="G$1" x="33.02" y="246.38"/>
<instance part="J2" gate="G$1" x="43.18" y="86.36"/>
<instance part="J3" gate="G$1" x="215.9" y="99.06"/>
<instance part="JP1" gate="G$1" x="195.58" y="127"/>
<instance part="Q1" gate="G$1" x="88.9" y="210.82"/>
<instance part="Q2" gate="G$1" x="172.72" y="248.92"/>
<instance part="R1" gate="G$1" x="30.48" y="220.98"/>
<instance part="R10" gate="G$1" x="83.82" y="104.14"/>
<instance part="R11" gate="G$1" x="111.76" y="104.14"/>
<instance part="R12" gate="G$1" x="139.7" y="104.14"/>
<instance part="R13" gate="G$1" x="167.64" y="104.14"/>
<instance part="R14" gate="G$1" x="241.3" y="104.14"/>
<instance part="R2" gate="G$1" x="58.42" y="220.98"/>
<instance part="R3" gate="G$1" x="139.7" y="220.98"/>
<instance part="R4" gate="G$1" x="167.64" y="220.98"/>
<instance part="R5" gate="G$1" x="111.76" y="220.98"/>
<instance part="R6" gate="G$1" x="58.42" y="129.54"/>
<instance part="R7" gate="G$1" x="86.36" y="129.54"/>
<instance part="R8" gate="G$1" x="139.7" y="129.54"/>
<instance part="R9" gate="G$1" x="167.64" y="129.54"/>
<instance part="U1" gate="G$1" x="220.98" y="254"/>
<instance part="U2" gate="G$1" x="43.18" y="160.02"/>
<instance part="U3" gate="G$1" x="292.1" y="233.68"/>
<instance part="Y1" gate="G$1" x="195.58" y="177.8"/>
</instances>
<busses>
</busses>
<nets>
<net name="GND" class="0">
<segment>
<pinref part="J1" gate="G$1" pin="4"/>
<pinref part="D2" gate="G$1" pin="A"/>
<pinref part="C2" gate="G$1" pin="2"/>
<pinref part="U1" gate="G$1" pin="GND"/>
<pinref part="C3" gate="G$1" pin="2"/>
<pinref part="C4" gate="G$1" pin="2"/>
<pinref part="C5" gate="G$1" pin="2"/>
<pinref part="C6" gate="G$1" pin="2"/>
<pinref part="C7" gate="G$1" pin="2"/>
<pinref part="C8" gate="G$1" pin="2"/>
<pinref part="C9" gate="G$1" pin="2"/>
<pinref part="C10" gate="G$1" pin="2"/>
<pinref part="C11" gate="G$1" pin="2"/>
<pinref part="C12" gate="G$1" pin="2"/>
<pinref part="U2" gate="G$1" pin="GND"/>
<pinref part="U2" gate="G$1" pin="VSS"/>
<pinref part="R4" gate="G$1" pin="2"/>
<pinref part="Q1" gate="G$1" pin="E"/>
<pinref part="D4" gate="G$1" pin="A"/>
<pinref part="D5" gate="G$1" pin="2"/>
<pinref part="J2" gate="G$1" pin="VSS"/>
<pinref part="J2" gate="G$1" pin="TL"/>
<pinref part="J2" gate="G$1" pin="L1"/>
<pinref part="J2" gate="G$1" pin="L2"/>
<pinref part="J2" gate="G$1" pin="L3"/>
<pinref part="J2" gate="G$1" pin="BR"/>
<pinref part="J3" gate="G$1" pin="GND"/>
<pinref part="U3" gate="G$1" pin="GND1"/>
<pinref part="U3" gate="G$1" pin="GND2"/>
<pinref part="U3" gate="G$1" pin="GND3"/>
<pinref part="U3" gate="G$1" pin="GND4"/>
<pinref part="U3" gate="G$1" pin="GND5"/>
<pinref part="U3" gate="G$1" pin="GND6"/>
<pinref part="U3" gate="G$1" pin="GND7"/>
<pinref part="U3" gate="G$1" pin="AGND"/>
<wire x1="25.4" y1="247.65" x2="22.86" y2="247.65" width="0.1524" layer="91"/>
<wire x1="22.86" y1="247.65" x2="22.86" y2="191" width="0.1524" layer="91"/>
<wire x1="109.22" y1="259.08" x2="106.68" y2="259.08" width="0.1524" layer="91"/>
<wire x1="106.68" y1="259.08" x2="106.68" y2="191" width="0.1524" layer="91"/>
<wire x1="147.32" y1="259.08" x2="149.86" y2="259.08" width="0.1524" layer="91"/>
<wire x1="149.86" y1="259.08" x2="149.86" y2="191" width="0.1524" layer="91"/>
<wire x1="220.98" y1="246.38" x2="220.98" y2="243.84" width="0.1524" layer="91"/>
<wire x1="220.98" y1="243.84" x2="220.98" y2="191" width="0.1524" layer="91"/>
<wire x1="88.9" y1="177.8" x2="91.44" y2="177.8" width="0.1524" layer="91"/>
<wire x1="91.44" y1="177.8" x2="91.44" y2="125" width="0.1524" layer="91"/>
<wire x1="116.84" y1="177.8" x2="119.38" y2="177.8" width="0.1524" layer="91"/>
<wire x1="119.38" y1="177.8" x2="119.38" y2="125" width="0.1524" layer="91"/>
<wire x1="144.78" y1="177.8" x2="147.32" y2="177.8" width="0.1524" layer="91"/>
<wire x1="147.32" y1="177.8" x2="147.32" y2="125" width="0.1524" layer="91"/>
<wire x1="172.72" y1="177.8" x2="175.26" y2="177.8" width="0.1524" layer="91"/>
<wire x1="175.26" y1="177.8" x2="175.26" y2="125" width="0.1524" layer="91"/>
<wire x1="228.6" y1="177.8" x2="231.14" y2="177.8" width="0.1524" layer="91"/>
<wire x1="231.14" y1="177.8" x2="231.14" y2="125" width="0.1524" layer="91"/>
<wire x1="35.56" y1="127" x2="38.1" y2="127" width="0.1524" layer="91"/>
<wire x1="38.1" y1="127" x2="38.1" y2="125" width="0.1524" layer="91"/>
<wire x1="228.6" y1="127" x2="231.14" y2="127" width="0.1524" layer="91"/>
<wire x1="231.14" y1="127" x2="231.14" y2="125" width="0.1524" layer="91"/>
<wire x1="35.56" y1="53.34" x2="38.1" y2="53.34" width="0.1524" layer="91"/>
<wire x1="38.1" y1="53.34" x2="38.1" y2="50" width="0.1524" layer="91"/>
<wire x1="63.5" y1="53.34" x2="66.04" y2="53.34" width="0.1524" layer="91"/>
<wire x1="66.04" y1="53.34" x2="66.04" y2="50" width="0.1524" layer="91"/>
<wire x1="63.5" y1="187.96" x2="66.04" y2="187.96" width="0.1524" layer="91"/>
<wire x1="66.04" y1="187.96" x2="66.04" y2="191" width="0.1524" layer="91"/>
<wire x1="25.4" y1="153.67" x2="22.86" y2="153.67" width="0.1524" layer="91"/>
<wire x1="22.86" y1="153.67" x2="22.86" y2="125" width="0.1524" layer="91"/>
<wire x1="60.96" y1="171.45" x2="63.5" y2="171.45" width="0.1524" layer="91"/>
<wire x1="63.5" y1="171.45" x2="63.5" y2="125" width="0.1524" layer="91"/>
<wire x1="172.72" y1="220.98" x2="175.26" y2="220.98" width="0.1524" layer="91"/>
<wire x1="175.26" y1="220.98" x2="175.26" y2="191" width="0.1524" layer="91"/>
<wire x1="91.44" y1="203.2" x2="91.44" y2="200.66" width="0.1524" layer="91"/>
<wire x1="91.44" y1="200.66" x2="91.44" y2="191" width="0.1524" layer="91"/>
<wire x1="25.4" y1="187.96" x2="22.86" y2="187.96" width="0.1524" layer="91"/>
<wire x1="22.86" y1="187.96" x2="22.86" y2="191" width="0.1524" layer="91"/>
<wire x1="116.84" y1="116.84" x2="116.84" y2="114.3" width="0.1524" layer="91"/>
<wire x1="116.84" y1="114.3" x2="124.46" y2="114.3" width="0.1524" layer="91"/>
<wire x1="124.46" y1="114.3" x2="124.46" y2="125" width="0.1524" layer="91"/>
<wire x1="25.4" y1="90.17" x2="22.86" y2="90.17" width="0.1524" layer="91"/>
<wire x1="22.86" y1="90.17" x2="22.86" y2="50" width="0.1524" layer="91"/>
<wire x1="60.96" y1="72.39" x2="63.5" y2="72.39" width="0.1524" layer="91"/>
<wire x1="63.5" y1="72.39" x2="63.5" y2="50" width="0.1524" layer="91"/>
<wire x1="60.96" y1="74.93" x2="63.5" y2="74.93" width="0.1524" layer="91"/>
<wire x1="63.5" y1="74.93" x2="63.5" y2="50" width="0.1524" layer="91"/>
<wire x1="60.96" y1="77.47" x2="63.5" y2="77.47" width="0.1524" layer="91"/>
<wire x1="63.5" y1="77.47" x2="63.5" y2="50" width="0.1524" layer="91"/>
<wire x1="60.96" y1="80.01" x2="63.5" y2="80.01" width="0.1524" layer="91"/>
<wire x1="63.5" y1="80.01" x2="63.5" y2="50" width="0.1524" layer="91"/>
<wire x1="60.96" y1="82.55" x2="63.5" y2="82.55" width="0.1524" layer="91"/>
<wire x1="63.5" y1="82.55" x2="63.5" y2="50" width="0.1524" layer="91"/>
<wire x1="208.28" y1="101.6" x2="205.74" y2="101.6" width="0.1524" layer="91"/>
<wire x1="205.74" y1="101.6" x2="205.74" y2="50" width="0.1524" layer="91"/>
<wire x1="269.24" y1="252.73" x2="266.7" y2="252.73" width="0.1524" layer="91"/>
<wire x1="266.7" y1="252.73" x2="266.7" y2="125" width="0.1524" layer="91"/>
<wire x1="269.24" y1="240.03" x2="266.7" y2="240.03" width="0.1524" layer="91"/>
<wire x1="266.7" y1="240.03" x2="266.7" y2="125" width="0.1524" layer="91"/>
<wire x1="269.24" y1="227.33" x2="266.7" y2="227.33" width="0.1524" layer="91"/>
<wire x1="266.7" y1="227.33" x2="266.7" y2="125" width="0.1524" layer="91"/>
<wire x1="269.24" y1="214.63" x2="266.7" y2="214.63" width="0.1524" layer="91"/>
<wire x1="266.7" y1="214.63" x2="266.7" y2="125" width="0.1524" layer="91"/>
<wire x1="314.96" y1="214.63" x2="317.5" y2="214.63" width="0.1524" layer="91"/>
<wire x1="317.5" y1="214.63" x2="317.5" y2="125" width="0.1524" layer="91"/>
<wire x1="314.96" y1="227.33" x2="317.5" y2="227.33" width="0.1524" layer="91"/>
<wire x1="317.5" y1="227.33" x2="317.5" y2="125" width="0.1524" layer="91"/>
<wire x1="314.96" y1="252.73" x2="317.5" y2="252.73" width="0.1524" layer="91"/>
<wire x1="317.5" y1="252.73" x2="317.5" y2="125" width="0.1524" layer="91"/>
<wire x1="314.96" y1="240.03" x2="317.5" y2="240.03" width="0.1524" layer="91"/>
<wire x1="317.5" y1="240.03" x2="317.5" y2="125" width="0.1524" layer="91"/>
<wire x1="6" y1="191" x2="220.98" y2="191" width="0.1524" layer="91"/>
<wire x1="6" y1="125" x2="317.5" y2="125" width="0.1524" layer="91"/>
<wire x1="6" y1="50" x2="205.74" y2="50" width="0.1524" layer="91"/>
<wire x1="6" y1="50" x2="6" y2="191" width="0.1524" layer="91"/>
<junction x="22.86" y="191"/>
<junction x="106.68" y="191"/>
<junction x="149.86" y="191"/>
<junction x="220.98" y="191"/>
<junction x="91.44" y="125"/>
<junction x="119.38" y="125"/>
<junction x="147.32" y="125"/>
<junction x="175.26" y="125"/>
<junction x="231.14" y="125"/>
<junction x="38.1" y="125"/>
<junction x="38.1" y="50"/>
<junction x="66.04" y="50"/>
<junction x="66.04" y="191"/>
<junction x="22.86" y="125"/>
<junction x="63.5" y="125"/>
<junction x="175.26" y="191"/>
<junction x="91.44" y="191"/>
<junction x="124.46" y="125"/>
<junction x="22.86" y="50"/>
<junction x="63.5" y="50"/>
<junction x="205.74" y="50"/>
<junction x="266.7" y="125"/>
<junction x="317.5" y="125"/>
<junction x="6" y="191"/>
<junction x="6" y="125"/>
<junction x="6" y="50"/>
</segment>
</net>
<net name="V5" class="0">
<segment>
<pinref part="U1" gate="G$1" pin="VOUT"/>
<pinref part="U2" gate="G$1" pin="VDDA"/>
<pinref part="C5" gate="G$1" pin="1"/>
<pinref part="C6" gate="G$1" pin="1"/>
<pinref part="D3" gate="G$1" pin="A"/>
<pinref part="R14" gate="G$1" pin="1"/>
<wire x1="233.68" y1="256.54" x2="236.22" y2="256.54" width="0.1524" layer="91"/>
<wire x1="236.22" y1="256.54" x2="236.22" y2="194" width="0.1524" layer="91"/>
<wire x1="60.96" y1="173.99" x2="63.5" y2="173.99" width="0.1524" layer="91"/>
<wire x1="63.5" y1="173.99" x2="63.5" y2="122" width="0.1524" layer="91"/>
<wire x1="134.62" y1="177.8" x2="132.08" y2="177.8" width="0.1524" layer="91"/>
<wire x1="132.08" y1="177.8" x2="132.08" y2="122" width="0.1524" layer="91"/>
<wire x1="162.56" y1="177.8" x2="160.02" y2="177.8" width="0.1524" layer="91"/>
<wire x1="160.02" y1="177.8" x2="160.02" y2="122" width="0.1524" layer="91"/>
<wire x1="251.46" y1="259.08" x2="248.92" y2="259.08" width="0.1524" layer="91"/>
<wire x1="248.92" y1="259.08" x2="248.92" y2="194" width="0.1524" layer="91"/>
<wire x1="236.22" y1="104.14" x2="233.68" y2="104.14" width="0.1524" layer="91"/>
<wire x1="233.68" y1="104.14" x2="233.68" y2="47" width="0.1524" layer="91"/>
<wire x1="9" y1="194" x2="248.92" y2="194" width="0.1524" layer="91"/>
<wire x1="9" y1="122" x2="160.02" y2="122" width="0.1524" layer="91"/>
<wire x1="9" y1="47" x2="233.68" y2="47" width="0.1524" layer="91"/>
<wire x1="9" y1="47" x2="9" y2="194" width="0.1524" layer="91"/>
<junction x="236.22" y="194"/>
<junction x="63.5" y="122"/>
<junction x="132.08" y="122"/>
<junction x="160.02" y="122"/>
<junction x="248.92" y="194"/>
<junction x="233.68" y="47"/>
<junction x="9" y="194"/>
<junction x="9" y="122"/>
<junction x="9" y="47"/>
</segment>
</net>
<net name="V3V3" class="0">
<segment>
<pinref part="U3" gate="G$1" pin="3V3OUT"/>
<pinref part="U2" gate="G$1" pin="VIO"/>
<pinref part="C3" gate="G$1" pin="1"/>
<pinref part="U2" gate="G$1" pin="VDD"/>
<pinref part="C4" gate="G$1" pin="1"/>
<pinref part="J2" gate="G$1" pin="VDD"/>
<pinref part="C10" gate="G$1" pin="1"/>
<pinref part="C11" gate="G$1" pin="1"/>
<pinref part="J3" gate="G$1" pin="VCC"/>
<pinref part="R6" gate="G$1" pin="2"/>
<pinref part="R7" gate="G$1" pin="2"/>
<pinref part="R10" gate="G$1" pin="2"/>
<pinref part="R11" gate="G$1" pin="2"/>
<pinref part="R12" gate="G$1" pin="2"/>
<pinref part="R13" gate="G$1" pin="2"/>
<wire x1="314.96" y1="247.65" x2="317.5" y2="247.65" width="0.1524" layer="91"/>
<wire x1="317.5" y1="247.65" x2="317.5" y2="119" width="0.1524" layer="91"/>
<wire x1="25.4" y1="176.53" x2="22.86" y2="176.53" width="0.1524" layer="91"/>
<wire x1="22.86" y1="176.53" x2="22.86" y2="119" width="0.1524" layer="91"/>
<wire x1="78.74" y1="177.8" x2="76.2" y2="177.8" width="0.1524" layer="91"/>
<wire x1="76.2" y1="177.8" x2="76.2" y2="119" width="0.1524" layer="91"/>
<wire x1="60.96" y1="153.67" x2="63.5" y2="153.67" width="0.1524" layer="91"/>
<wire x1="63.5" y1="153.67" x2="63.5" y2="119" width="0.1524" layer="91"/>
<wire x1="106.68" y1="177.8" x2="104.14" y2="177.8" width="0.1524" layer="91"/>
<wire x1="104.14" y1="177.8" x2="104.14" y2="119" width="0.1524" layer="91"/>
<wire x1="25.4" y1="95.25" x2="22.86" y2="95.25" width="0.1524" layer="91"/>
<wire x1="22.86" y1="95.25" x2="22.86" y2="44" width="0.1524" layer="91"/>
<wire x1="25.4" y1="53.34" x2="22.86" y2="53.34" width="0.1524" layer="91"/>
<wire x1="22.86" y1="53.34" x2="22.86" y2="44" width="0.1524" layer="91"/>
<wire x1="53.34" y1="53.34" x2="50.8" y2="53.34" width="0.1524" layer="91"/>
<wire x1="50.8" y1="53.34" x2="50.8" y2="44" width="0.1524" layer="91"/>
<wire x1="208.28" y1="104.14" x2="205.74" y2="104.14" width="0.1524" layer="91"/>
<wire x1="205.74" y1="104.14" x2="205.74" y2="44" width="0.1524" layer="91"/>
<wire x1="63.5" y1="129.54" x2="66.04" y2="129.54" width="0.1524" layer="91"/>
<wire x1="66.04" y1="129.54" x2="66.04" y2="119" width="0.1524" layer="91"/>
<wire x1="91.44" y1="129.54" x2="93.98" y2="129.54" width="0.1524" layer="91"/>
<wire x1="93.98" y1="129.54" x2="93.98" y2="119" width="0.1524" layer="91"/>
<wire x1="88.9" y1="104.14" x2="91.44" y2="104.14" width="0.1524" layer="91"/>
<wire x1="91.44" y1="104.14" x2="91.44" y2="44" width="0.1524" layer="91"/>
<wire x1="116.84" y1="104.14" x2="119.38" y2="104.14" width="0.1524" layer="91"/>
<wire x1="119.38" y1="104.14" x2="119.38" y2="44" width="0.1524" layer="91"/>
<wire x1="144.78" y1="104.14" x2="147.32" y2="104.14" width="0.1524" layer="91"/>
<wire x1="147.32" y1="104.14" x2="147.32" y2="44" width="0.1524" layer="91"/>
<wire x1="172.72" y1="104.14" x2="175.26" y2="104.14" width="0.1524" layer="91"/>
<wire x1="175.26" y1="104.14" x2="175.26" y2="44" width="0.1524" layer="91"/>
<wire x1="12" y1="119" x2="317.5" y2="119" width="0.1524" layer="91"/>
<wire x1="12" y1="44" x2="205.74" y2="44" width="0.1524" layer="91"/>
<wire x1="12" y1="44" x2="12" y2="119" width="0.1524" layer="91"/>
<junction x="317.5" y="119"/>
<junction x="22.86" y="119"/>
<junction x="76.2" y="119"/>
<junction x="63.5" y="119"/>
<junction x="104.14" y="119"/>
<junction x="22.86" y="44"/>
<junction x="50.8" y="44"/>
<junction x="205.74" y="44"/>
<junction x="66.04" y="119"/>
<junction x="93.98" y="119"/>
<junction x="91.44" y="44"/>
<junction x="119.38" y="44"/>
<junction x="147.32" y="44"/>
<junction x="175.26" y="44"/>
<junction x="12" y="119"/>
<junction x="12" y="44"/>
</segment>
</net>
<net name="V12P" class="0">
<segment>
<pinref part="D1" gate="G$1" pin="K"/>
<pinref part="D2" gate="G$1" pin="K"/>
<pinref part="C2" gate="G$1" pin="1"/>
<pinref part="R1" gate="G$1" pin="1"/>
<pinref part="Q2" gate="G$1" pin="S"/>
<wire x1="91.44" y1="259.08" x2="93.98" y2="259.08" width="0.1524" layer="91"/>
<wire x1="93.98" y1="259.08" x2="93.98" y2="197" width="0.1524" layer="91"/>
<wire x1="119.38" y1="259.08" x2="121.92" y2="259.08" width="0.1524" layer="91"/>
<wire x1="121.92" y1="259.08" x2="121.92" y2="197" width="0.1524" layer="91"/>
<wire x1="137.16" y1="259.08" x2="134.62" y2="259.08" width="0.1524" layer="91"/>
<wire x1="134.62" y1="259.08" x2="134.62" y2="197" width="0.1524" layer="91"/>
<wire x1="25.4" y1="220.98" x2="22.86" y2="220.98" width="0.1524" layer="91"/>
<wire x1="22.86" y1="220.98" x2="22.86" y2="197" width="0.1524" layer="91"/>
<wire x1="172.72" y1="256.54" x2="172.72" y2="259.08" width="0.1524" layer="91"/>
<wire x1="172.72" y1="259.08" x2="180.34" y2="259.08" width="0.1524" layer="91"/>
<wire x1="180.34" y1="259.08" x2="180.34" y2="197" width="0.1524" layer="91"/>
<wire x1="22.86" y1="197" x2="180.34" y2="197" width="0.1524" layer="91"/>
<junction x="93.98" y="197"/>
<junction x="121.92" y="197"/>
<junction x="134.62" y="197"/>
<junction x="22.86" y="197"/>
<junction x="180.34" y="197"/>
</segment>
</net>
<net name="V12_SW" class="0">
<segment>
<pinref part="Q2" gate="G$1" pin="D"/>
<pinref part="U1" gate="G$1" pin="VIN"/>
<wire x1="172.72" y1="241.3" x2="172.72" y2="238.76" width="0.1524" layer="91"/>
<wire x1="172.72" y1="238.76" x2="172.72" y2="200" width="0.1524" layer="91"/>
<wire x1="208.28" y1="256.54" x2="205.74" y2="256.54" width="0.1524" layer="91"/>
<wire x1="205.74" y1="256.54" x2="205.74" y2="200" width="0.1524" layer="91"/>
<wire x1="172.72" y1="200" x2="205.74" y2="200" width="0.1524" layer="91"/>
<junction x="172.72" y="200"/>
<junction x="205.74" y="200"/>
</segment>
</net>
<net name="V12_RAW" class="0">
<segment>
<pinref part="J1" gate="G$1" pin="8"/>
<pinref part="F1" gate="G$1" pin="1"/>
<wire x1="25.4" y1="237.49" x2="22.86" y2="237.49" width="0.1524" layer="91"/>
<wire x1="22.86" y1="237.49" x2="50.8" y2="237.49" width="0.1524" layer="91"/>
<wire x1="50.8" y1="237.49" x2="50.8" y2="259.08" width="0.1524" layer="91"/>
<wire x1="50.8" y1="259.08" x2="53.34" y2="259.08" width="0.1524" layer="91"/>
<label x="51.3" y="259.88" size="1.27" layer="95"/>
</segment>
</net>
<net name="V12_FUSED" class="0">
<segment>
<pinref part="F1" gate="G$1" pin="2"/>
<pinref part="D1" gate="G$1" pin="A"/>
<wire x1="63.5" y1="259.08" x2="66.04" y2="259.08" width="0.1524" layer="91"/>
<wire x1="66.04" y1="259.08" x2="78.74" y2="259.08" width="0.1524" layer="91"/>
<wire x1="78.74" y1="259.08" x2="81.28" y2="259.08" width="0.1524" layer="91"/>
<label x="79.24" y="259.88" size="1.27" layer="95"/>
</segment>
</net>
<net name="IG_RAW" class="0">
<segment>
<pinref part="J1" gate="G$1" pin="3"/>
<pinref part="R3" gate="G$1" pin="1"/>
<wire x1="25.4" y1="250.19" x2="22.86" y2="250.19" width="0.1524" layer="91"/>
<wire x1="22.86" y1="250.19" x2="132.08" y2="250.19" width="0.1524" layer="91"/>
<wire x1="132.08" y1="250.19" x2="132.08" y2="220.98" width="0.1524" layer="91"/>
<wire x1="132.08" y1="220.98" x2="134.62" y2="220.98" width="0.1524" layer="91"/>
<label x="132.58" y="221.78" size="1.27" layer="95"/>
</segment>
</net>
<net name="IG_DIV" class="0">
<segment>
<pinref part="R3" gate="G$1" pin="2"/>
<pinref part="R4" gate="G$1" pin="1"/>
<pinref part="D4" gate="G$1" pin="K"/>
<pinref part="C12" gate="G$1" pin="1"/>
<pinref part="R5" gate="G$1" pin="1"/>
<pinref part="U3" gate="G$1" pin="GP28_ADC2"/>
<wire x1="144.78" y1="220.98" x2="147.32" y2="220.98" width="0.1524" layer="91"/>
<wire x1="147.32" y1="220.98" x2="160.02" y2="220.98" width="0.1524" layer="91"/>
<wire x1="160.02" y1="220.98" x2="162.56" y2="220.98" width="0.1524" layer="91"/>
<wire x1="162.56" y1="220.98" x2="160.02" y2="220.98" width="0.1524" layer="91"/>
<wire x1="160.02" y1="220.98" x2="38.1" y2="220.98" width="0.1524" layer="91"/>
<wire x1="38.1" y1="220.98" x2="38.1" y2="187.96" width="0.1524" layer="91"/>
<wire x1="38.1" y1="187.96" x2="35.56" y2="187.96" width="0.1524" layer="91"/>
<wire x1="35.56" y1="187.96" x2="38.1" y2="187.96" width="0.1524" layer="91"/>
<wire x1="38.1" y1="187.96" x2="50.8" y2="187.96" width="0.1524" layer="91"/>
<wire x1="50.8" y1="187.96" x2="53.34" y2="187.96" width="0.1524" layer="91"/>
<wire x1="53.34" y1="187.96" x2="50.8" y2="187.96" width="0.1524" layer="91"/>
<wire x1="50.8" y1="187.96" x2="104.14" y2="187.96" width="0.1524" layer="91"/>
<wire x1="104.14" y1="187.96" x2="104.14" y2="220.98" width="0.1524" layer="91"/>
<wire x1="104.14" y1="220.98" x2="106.68" y2="220.98" width="0.1524" layer="91"/>
<wire x1="106.68" y1="220.98" x2="104.14" y2="220.98" width="0.1524" layer="91"/>
<wire x1="104.14" y1="220.98" x2="317.5" y2="220.98" width="0.1524" layer="91"/>
<wire x1="317.5" y1="220.98" x2="317.5" y2="242.57" width="0.1524" layer="91"/>
<wire x1="317.5" y1="242.57" x2="314.96" y2="242.57" width="0.1524" layer="91"/>
<label x="51.3" y="188.76" size="1.27" layer="95"/>
</segment>
</net>
<net name="Q1_B" class="0">
<segment>
<pinref part="R5" gate="G$1" pin="2"/>
<pinref part="Q1" gate="G$1" pin="B"/>
<wire x1="116.84" y1="220.98" x2="119.38" y2="220.98" width="0.1524" layer="91"/>
<wire x1="119.38" y1="220.98" x2="78.74" y2="220.98" width="0.1524" layer="91"/>
<wire x1="78.74" y1="220.98" x2="78.74" y2="210.82" width="0.1524" layer="91"/>
<wire x1="78.74" y1="210.82" x2="81.28" y2="210.82" width="0.1524" layer="91"/>
<label x="79.24" y="211.62" size="1.27" layer="95"/>
</segment>
</net>
<net name="Q2_G" class="0">
<segment>
<pinref part="R1" gate="G$1" pin="2"/>
<pinref part="Q2" gate="G$1" pin="G"/>
<pinref part="R2" gate="G$1" pin="1"/>
<wire x1="35.56" y1="220.98" x2="38.1" y2="220.98" width="0.1524" layer="91"/>
<wire x1="38.1" y1="220.98" x2="162.56" y2="220.98" width="0.1524" layer="91"/>
<wire x1="162.56" y1="220.98" x2="162.56" y2="248.92" width="0.1524" layer="91"/>
<wire x1="162.56" y1="248.92" x2="165.1" y2="248.92" width="0.1524" layer="91"/>
<wire x1="165.1" y1="248.92" x2="162.56" y2="248.92" width="0.1524" layer="91"/>
<wire x1="162.56" y1="248.92" x2="50.8" y2="248.92" width="0.1524" layer="91"/>
<wire x1="50.8" y1="248.92" x2="50.8" y2="220.98" width="0.1524" layer="91"/>
<wire x1="50.8" y1="220.98" x2="53.34" y2="220.98" width="0.1524" layer="91"/>
<label x="163.06" y="249.72" size="1.27" layer="95"/>
</segment>
</net>
<net name="Q1_C" class="0">
<segment>
<pinref part="R2" gate="G$1" pin="2"/>
<pinref part="Q1" gate="G$1" pin="C"/>
<wire x1="63.5" y1="220.98" x2="66.04" y2="220.98" width="0.1524" layer="91"/>
<wire x1="66.04" y1="220.98" x2="91.44" y2="220.98" width="0.1524" layer="91"/>
<wire x1="91.44" y1="220.98" x2="91.44" y2="218.44" width="0.1524" layer="91"/>
<label x="91.94" y="221.78" size="1.27" layer="95"/>
</segment>
</net>
<net name="VSYS" class="0">
<segment>
<pinref part="D3" gate="G$1" pin="K"/>
<pinref part="U3" gate="G$1" pin="VSYS"/>
<wire x1="261.62" y1="259.08" x2="264.16" y2="259.08" width="0.1524" layer="91"/>
<wire x1="264.16" y1="259.08" x2="317.5" y2="259.08" width="0.1524" layer="91"/>
<wire x1="317.5" y1="259.08" x2="317.5" y2="255.27" width="0.1524" layer="91"/>
<wire x1="317.5" y1="255.27" x2="314.96" y2="255.27" width="0.1524" layer="91"/>
<label x="318" y="256.07" size="1.27" layer="95"/>
</segment>
</net>
<net name="CANH" class="0">
<segment>
<pinref part="J1" gate="G$1" pin="6"/>
<pinref part="D5" gate="G$1" pin="1"/>
<pinref part="U2" gate="G$1" pin="CANH"/>
<pinref part="R8" gate="G$1" pin="1"/>
<wire x1="25.4" y1="242.57" x2="22.86" y2="242.57" width="0.1524" layer="91"/>
<wire x1="22.86" y1="242.57" x2="106.68" y2="242.57" width="0.1524" layer="91"/>
<wire x1="106.68" y1="242.57" x2="106.68" y2="127" width="0.1524" layer="91"/>
<wire x1="106.68" y1="127" x2="109.22" y2="127" width="0.1524" layer="91"/>
<wire x1="109.22" y1="127" x2="106.68" y2="127" width="0.1524" layer="91"/>
<wire x1="106.68" y1="127" x2="22.86" y2="127" width="0.1524" layer="91"/>
<wire x1="22.86" y1="127" x2="22.86" y2="168.91" width="0.1524" layer="91"/>
<wire x1="22.86" y1="168.91" x2="25.4" y2="168.91" width="0.1524" layer="91"/>
<wire x1="25.4" y1="168.91" x2="22.86" y2="168.91" width="0.1524" layer="91"/>
<wire x1="22.86" y1="168.91" x2="132.08" y2="168.91" width="0.1524" layer="91"/>
<wire x1="132.08" y1="168.91" x2="132.08" y2="129.54" width="0.1524" layer="91"/>
<wire x1="132.08" y1="129.54" x2="134.62" y2="129.54" width="0.1524" layer="91"/>
<label x="23.36" y="169.71" size="1.27" layer="95"/>
</segment>
</net>
<net name="CANL" class="0">
<segment>
<pinref part="J1" gate="G$1" pin="5"/>
<pinref part="D5" gate="G$1" pin="3"/>
<pinref part="U2" gate="G$1" pin="CANL"/>
<pinref part="R9" gate="G$1" pin="1"/>
<wire x1="25.4" y1="245.11" x2="22.86" y2="245.11" width="0.1524" layer="91"/>
<wire x1="22.86" y1="245.11" x2="106.68" y2="245.11" width="0.1524" layer="91"/>
<wire x1="106.68" y1="245.11" x2="106.68" y2="121.92" width="0.1524" layer="91"/>
<wire x1="106.68" y1="121.92" x2="109.22" y2="121.92" width="0.1524" layer="91"/>
<wire x1="109.22" y1="121.92" x2="106.68" y2="121.92" width="0.1524" layer="91"/>
<wire x1="106.68" y1="121.92" x2="22.86" y2="121.92" width="0.1524" layer="91"/>
<wire x1="22.86" y1="121.92" x2="22.86" y2="171.45" width="0.1524" layer="91"/>
<wire x1="22.86" y1="171.45" x2="25.4" y2="171.45" width="0.1524" layer="91"/>
<wire x1="25.4" y1="171.45" x2="22.86" y2="171.45" width="0.1524" layer="91"/>
<wire x1="22.86" y1="171.45" x2="160.02" y2="171.45" width="0.1524" layer="91"/>
<wire x1="160.02" y1="171.45" x2="160.02" y2="129.54" width="0.1524" layer="91"/>
<wire x1="160.02" y1="129.54" x2="162.56" y2="129.54" width="0.1524" layer="91"/>
<label x="23.36" y="172.25" size="1.27" layer="95"/>
</segment>
</net>
<net name="CAN_TERM_MID" class="0">
<segment>
<pinref part="R8" gate="G$1" pin="2"/>
<pinref part="R9" gate="G$1" pin="2"/>
<pinref part="JP1" gate="G$1" pin="1"/>
<wire x1="144.78" y1="129.54" x2="147.32" y2="129.54" width="0.1524" layer="91"/>
<wire x1="147.32" y1="129.54" x2="175.26" y2="129.54" width="0.1524" layer="91"/>
<wire x1="175.26" y1="129.54" x2="172.72" y2="129.54" width="0.1524" layer="91"/>
<wire x1="172.72" y1="129.54" x2="175.26" y2="129.54" width="0.1524" layer="91"/>
<wire x1="175.26" y1="129.54" x2="187.96" y2="129.54" width="0.1524" layer="91"/>
<wire x1="187.96" y1="129.54" x2="187.96" y2="127" width="0.1524" layer="91"/>
<wire x1="187.96" y1="127" x2="190.5" y2="127" width="0.1524" layer="91"/>
<label x="175.76" y="130.34" size="1.27" layer="95"/>
</segment>
</net>
<net name="CAN_TERM_C" class="0">
<segment>
<pinref part="JP1" gate="G$1" pin="2"/>
<pinref part="C9" gate="G$1" pin="1"/>
<wire x1="200.66" y1="127" x2="203.2" y2="127" width="0.1524" layer="91"/>
<wire x1="203.2" y1="127" x2="215.9" y2="127" width="0.1524" layer="91"/>
<wire x1="215.9" y1="127" x2="218.44" y2="127" width="0.1524" layer="91"/>
<label x="216.4" y="127.8" size="1.27" layer="95"/>
</segment>
</net>
<net name="TXCAN" class="0">
<segment>
<pinref part="U2" gate="G$1" pin="TXCAN"/>
<pinref part="U2" gate="G$1" pin="TXD"/>
<wire x1="60.96" y1="156.21" x2="63.5" y2="156.21" width="0.1524" layer="91"/>
<wire x1="63.5" y1="156.21" x2="63.5" y2="166.37" width="0.1524" layer="91"/>
<wire x1="63.5" y1="166.37" x2="60.96" y2="166.37" width="0.1524" layer="91"/>
<label x="64" y="167.17" size="1.27" layer="95"/>
</segment>
</net>
<net name="RXCAN" class="0">
<segment>
<pinref part="U2" gate="G$1" pin="RXCAN"/>
<pinref part="U2" gate="G$1" pin="RXD"/>
<wire x1="60.96" y1="158.75" x2="63.5" y2="158.75" width="0.1524" layer="91"/>
<wire x1="63.5" y1="158.75" x2="63.5" y2="176.53" width="0.1524" layer="91"/>
<wire x1="63.5" y1="176.53" x2="60.96" y2="176.53" width="0.1524" layer="91"/>
<label x="64" y="177.33" size="1.27" layer="95"/>
</segment>
</net>
<net name="TXNRTS" class="0">
<segment>
<pinref part="U2" gate="G$1" pin="TX0RTS"/>
<pinref part="U2" gate="G$1" pin="TX1RTS"/>
<pinref part="U2" gate="G$1" pin="TX2RTS"/>
<pinref part="R7" gate="G$1" pin="1"/>
<wire x1="60.96" y1="163.83" x2="63.5" y2="163.83" width="0.1524" layer="91"/>
<wire x1="63.5" y1="163.83" x2="22.86" y2="163.83" width="0.1524" layer="91"/>
<wire x1="22.86" y1="163.83" x2="25.4" y2="163.83" width="0.1524" layer="91"/>
<wire x1="25.4" y1="163.83" x2="22.86" y2="163.83" width="0.1524" layer="91"/>
<wire x1="22.86" y1="163.83" x2="22.86" y2="161.29" width="0.1524" layer="91"/>
<wire x1="22.86" y1="161.29" x2="25.4" y2="161.29" width="0.1524" layer="91"/>
<wire x1="25.4" y1="161.29" x2="22.86" y2="161.29" width="0.1524" layer="91"/>
<wire x1="22.86" y1="161.29" x2="78.74" y2="161.29" width="0.1524" layer="91"/>
<wire x1="78.74" y1="161.29" x2="78.74" y2="129.54" width="0.1524" layer="91"/>
<wire x1="78.74" y1="129.54" x2="81.28" y2="129.54" width="0.1524" layer="91"/>
<label x="23.36" y="162.09" size="1.27" layer="95"/>
</segment>
</net>
<net name="CAN_RESET" class="0">
<segment>
<pinref part="U2" gate="G$1" pin="RESET"/>
<pinref part="R6" gate="G$1" pin="1"/>
<pinref part="U3" gate="G$1" pin="GP20"/>
<wire x1="60.96" y1="151.13" x2="63.5" y2="151.13" width="0.1524" layer="91"/>
<wire x1="63.5" y1="151.13" x2="50.8" y2="151.13" width="0.1524" layer="91"/>
<wire x1="50.8" y1="151.13" x2="50.8" y2="129.54" width="0.1524" layer="91"/>
<wire x1="50.8" y1="129.54" x2="53.34" y2="129.54" width="0.1524" layer="91"/>
<wire x1="53.34" y1="129.54" x2="50.8" y2="129.54" width="0.1524" layer="91"/>
<wire x1="50.8" y1="129.54" x2="317.5" y2="129.54" width="0.1524" layer="91"/>
<wire x1="317.5" y1="129.54" x2="317.5" y2="222.25" width="0.1524" layer="91"/>
<wire x1="317.5" y1="222.25" x2="314.96" y2="222.25" width="0.1524" layer="91"/>
<label x="51.3" y="130.34" size="1.27" layer="95"/>
</segment>
</net>
<net name="XTAL1" class="0">
<segment>
<pinref part="U2" gate="G$1" pin="OSC1"/>
<pinref part="Y1" gate="G$1" pin="1"/>
<pinref part="C7" gate="G$1" pin="1"/>
<wire x1="25.4" y1="156.21" x2="22.86" y2="156.21" width="0.1524" layer="91"/>
<wire x1="22.86" y1="156.21" x2="187.96" y2="156.21" width="0.1524" layer="91"/>
<wire x1="187.96" y1="156.21" x2="187.96" y2="177.8" width="0.1524" layer="91"/>
<wire x1="187.96" y1="177.8" x2="190.5" y2="177.8" width="0.1524" layer="91"/>
<wire x1="190.5" y1="177.8" x2="187.96" y2="177.8" width="0.1524" layer="91"/>
<wire x1="187.96" y1="177.8" x2="215.9" y2="177.8" width="0.1524" layer="91"/>
<wire x1="215.9" y1="177.8" x2="218.44" y2="177.8" width="0.1524" layer="91"/>
<label x="188.46" y="178.6" size="1.27" layer="95"/>
</segment>
</net>
<net name="XTAL2" class="0">
<segment>
<pinref part="U2" gate="G$1" pin="OSC2"/>
<pinref part="Y1" gate="G$1" pin="2"/>
<pinref part="C8" gate="G$1" pin="1"/>
<wire x1="25.4" y1="158.75" x2="22.86" y2="158.75" width="0.1524" layer="91"/>
<wire x1="22.86" y1="158.75" x2="203.2" y2="158.75" width="0.1524" layer="91"/>
<wire x1="203.2" y1="158.75" x2="203.2" y2="177.8" width="0.1524" layer="91"/>
<wire x1="203.2" y1="177.8" x2="200.66" y2="177.8" width="0.1524" layer="91"/>
<wire x1="200.66" y1="177.8" x2="203.2" y2="177.8" width="0.1524" layer="91"/>
<wire x1="203.2" y1="177.8" x2="22.86" y2="177.8" width="0.1524" layer="91"/>
<wire x1="22.86" y1="177.8" x2="22.86" y2="127" width="0.1524" layer="91"/>
<wire x1="22.86" y1="127" x2="25.4" y2="127" width="0.1524" layer="91"/>
<label x="203.7" y="178.6" size="1.27" layer="95"/>
</segment>
</net>
<net name="CAN_SCK" class="0">
<segment>
<pinref part="U3" gate="G$1" pin="GP10"/>
<pinref part="U2" gate="G$1" pin="SCK"/>
<wire x1="269.24" y1="224.79" x2="266.7" y2="224.79" width="0.1524" layer="91"/>
<wire x1="266.7" y1="224.79" x2="22.86" y2="224.79" width="0.1524" layer="91"/>
<wire x1="22.86" y1="224.79" x2="22.86" y2="143.51" width="0.1524" layer="91"/>
<wire x1="22.86" y1="143.51" x2="25.4" y2="143.51" width="0.1524" layer="91"/>
<label x="23.36" y="144.31" size="1.27" layer="95"/>
</segment>
</net>
<net name="CAN_MOSI" class="0">
<segment>
<pinref part="U3" gate="G$1" pin="GP11"/>
<pinref part="U2" gate="G$1" pin="SI"/>
<wire x1="269.24" y1="222.25" x2="266.7" y2="222.25" width="0.1524" layer="91"/>
<wire x1="266.7" y1="222.25" x2="63.5" y2="222.25" width="0.1524" layer="91"/>
<wire x1="63.5" y1="222.25" x2="63.5" y2="143.51" width="0.1524" layer="91"/>
<wire x1="63.5" y1="143.51" x2="60.96" y2="143.51" width="0.1524" layer="91"/>
<label x="64" y="144.31" size="1.27" layer="95"/>
</segment>
</net>
<net name="CAN_MISO" class="0">
<segment>
<pinref part="U3" gate="G$1" pin="GP12"/>
<pinref part="U2" gate="G$1" pin="SO"/>
<wire x1="269.24" y1="219.71" x2="266.7" y2="219.71" width="0.1524" layer="91"/>
<wire x1="266.7" y1="219.71" x2="63.5" y2="219.71" width="0.1524" layer="91"/>
<wire x1="63.5" y1="219.71" x2="63.5" y2="146.05" width="0.1524" layer="91"/>
<wire x1="63.5" y1="146.05" x2="60.96" y2="146.05" width="0.1524" layer="91"/>
<label x="64" y="146.85" size="1.27" layer="95"/>
</segment>
</net>
<net name="CAN_CS" class="0">
<segment>
<pinref part="U3" gate="G$1" pin="GP13"/>
<pinref part="U2" gate="G$1" pin="CS"/>
<wire x1="269.24" y1="217.17" x2="266.7" y2="217.17" width="0.1524" layer="91"/>
<wire x1="266.7" y1="217.17" x2="63.5" y2="217.17" width="0.1524" layer="91"/>
<wire x1="63.5" y1="217.17" x2="63.5" y2="148.59" width="0.1524" layer="91"/>
<wire x1="63.5" y1="148.59" x2="60.96" y2="148.59" width="0.1524" layer="91"/>
<label x="64" y="149.39" size="1.27" layer="95"/>
</segment>
</net>
<net name="CAN_INT" class="0">
<segment>
<pinref part="U3" gate="G$1" pin="GP14"/>
<pinref part="U2" gate="G$1" pin="INT"/>
<wire x1="269.24" y1="212.09" x2="266.7" y2="212.09" width="0.1524" layer="91"/>
<wire x1="266.7" y1="212.09" x2="22.86" y2="212.09" width="0.1524" layer="91"/>
<wire x1="22.86" y1="212.09" x2="22.86" y2="146.05" width="0.1524" layer="91"/>
<wire x1="22.86" y1="146.05" x2="25.4" y2="146.05" width="0.1524" layer="91"/>
<label x="23.36" y="146.85" size="1.27" layer="95"/>
</segment>
</net>
<net name="CAN_STBY" class="0">
<segment>
<pinref part="U3" gate="G$1" pin="GP15"/>
<pinref part="U2" gate="G$1" pin="STBY"/>
<wire x1="269.24" y1="209.55" x2="266.7" y2="209.55" width="0.1524" layer="91"/>
<wire x1="266.7" y1="209.55" x2="22.86" y2="209.55" width="0.1524" layer="91"/>
<wire x1="22.86" y1="209.55" x2="22.86" y2="166.37" width="0.1524" layer="91"/>
<wire x1="22.86" y1="166.37" x2="25.4" y2="166.37" width="0.1524" layer="91"/>
<label x="23.36" y="167.17" size="1.27" layer="95"/>
</segment>
</net>
<net name="SD_CS" class="0">
<segment>
<pinref part="J2" gate="G$1" pin="CD_DAT3"/>
<pinref part="R10" gate="G$1" pin="1"/>
<pinref part="U3" gate="G$1" pin="GP17"/>
<wire x1="25.4" y1="100.33" x2="22.86" y2="100.33" width="0.1524" layer="91"/>
<wire x1="22.86" y1="100.33" x2="76.2" y2="100.33" width="0.1524" layer="91"/>
<wire x1="76.2" y1="100.33" x2="76.2" y2="104.14" width="0.1524" layer="91"/>
<wire x1="76.2" y1="104.14" x2="78.74" y2="104.14" width="0.1524" layer="91"/>
<wire x1="78.74" y1="104.14" x2="76.2" y2="104.14" width="0.1524" layer="91"/>
<wire x1="76.2" y1="104.14" x2="317.5" y2="104.14" width="0.1524" layer="91"/>
<wire x1="317.5" y1="104.14" x2="317.5" y2="212.09" width="0.1524" layer="91"/>
<wire x1="317.5" y1="212.09" x2="314.96" y2="212.09" width="0.1524" layer="91"/>
<label x="76.7" y="104.94" size="1.27" layer="95"/>
</segment>
</net>
<net name="SD_MOSI" class="0">
<segment>
<pinref part="J2" gate="G$1" pin="CMD"/>
<pinref part="U3" gate="G$1" pin="GP19"/>
<wire x1="25.4" y1="97.79" x2="22.86" y2="97.79" width="0.1524" layer="91"/>
<wire x1="22.86" y1="97.79" x2="317.5" y2="97.79" width="0.1524" layer="91"/>
<wire x1="317.5" y1="97.79" x2="317.5" y2="219.71" width="0.1524" layer="91"/>
<wire x1="317.5" y1="219.71" x2="314.96" y2="219.71" width="0.1524" layer="91"/>
<label x="318" y="220.51" size="1.27" layer="95"/>
</segment>
</net>
<net name="SD_SCK" class="0">
<segment>
<pinref part="J2" gate="G$1" pin="CLK"/>
<pinref part="U3" gate="G$1" pin="GP18"/>
<wire x1="25.4" y1="92.71" x2="22.86" y2="92.71" width="0.1524" layer="91"/>
<wire x1="22.86" y1="92.71" x2="317.5" y2="92.71" width="0.1524" layer="91"/>
<wire x1="317.5" y1="92.71" x2="317.5" y2="217.17" width="0.1524" layer="91"/>
<wire x1="317.5" y1="217.17" x2="314.96" y2="217.17" width="0.1524" layer="91"/>
<label x="318" y="217.97" size="1.27" layer="95"/>
</segment>
</net>
<net name="SD_MISO" class="0">
<segment>
<pinref part="J2" gate="G$1" pin="DAT0"/>
<pinref part="R11" gate="G$1" pin="1"/>
<pinref part="U3" gate="G$1" pin="GP16"/>
<wire x1="25.4" y1="87.63" x2="22.86" y2="87.63" width="0.1524" layer="91"/>
<wire x1="22.86" y1="87.63" x2="104.14" y2="87.63" width="0.1524" layer="91"/>
<wire x1="104.14" y1="87.63" x2="104.14" y2="104.14" width="0.1524" layer="91"/>
<wire x1="104.14" y1="104.14" x2="106.68" y2="104.14" width="0.1524" layer="91"/>
<wire x1="106.68" y1="104.14" x2="104.14" y2="104.14" width="0.1524" layer="91"/>
<wire x1="104.14" y1="104.14" x2="317.5" y2="104.14" width="0.1524" layer="91"/>
<wire x1="317.5" y1="104.14" x2="317.5" y2="209.55" width="0.1524" layer="91"/>
<wire x1="317.5" y1="209.55" x2="314.96" y2="209.55" width="0.1524" layer="91"/>
<label x="104.64" y="104.94" size="1.27" layer="95"/>
</segment>
</net>
<net name="SD_DAT12" class="0">
<segment>
<pinref part="J2" gate="G$1" pin="DAT1"/>
<pinref part="J2" gate="G$1" pin="DAT2"/>
<pinref part="R12" gate="G$1" pin="1"/>
<wire x1="25.4" y1="85.09" x2="22.86" y2="85.09" width="0.1524" layer="91"/>
<wire x1="22.86" y1="85.09" x2="22.86" y2="102.87" width="0.1524" layer="91"/>
<wire x1="22.86" y1="102.87" x2="25.4" y2="102.87" width="0.1524" layer="91"/>
<wire x1="25.4" y1="102.87" x2="22.86" y2="102.87" width="0.1524" layer="91"/>
<wire x1="22.86" y1="102.87" x2="132.08" y2="102.87" width="0.1524" layer="91"/>
<wire x1="132.08" y1="102.87" x2="132.08" y2="104.14" width="0.1524" layer="91"/>
<wire x1="132.08" y1="104.14" x2="134.62" y2="104.14" width="0.1524" layer="91"/>
<label x="23.36" y="103.67" size="1.27" layer="95"/>
</segment>
</net>
<net name="SD_CD" class="0">
<segment>
<pinref part="J2" gate="G$1" pin="TR"/>
<pinref part="R13" gate="G$1" pin="1"/>
<pinref part="U3" gate="G$1" pin="GP21"/>
<wire x1="60.96" y1="69.85" x2="63.5" y2="69.85" width="0.1524" layer="91"/>
<wire x1="63.5" y1="69.85" x2="160.02" y2="69.85" width="0.1524" layer="91"/>
<wire x1="160.02" y1="69.85" x2="160.02" y2="104.14" width="0.1524" layer="91"/>
<wire x1="160.02" y1="104.14" x2="162.56" y2="104.14" width="0.1524" layer="91"/>
<wire x1="162.56" y1="104.14" x2="160.02" y2="104.14" width="0.1524" layer="91"/>
<wire x1="160.02" y1="104.14" x2="317.5" y2="104.14" width="0.1524" layer="91"/>
<wire x1="317.5" y1="104.14" x2="317.5" y2="224.79" width="0.1524" layer="91"/>
<wire x1="317.5" y1="224.79" x2="314.96" y2="224.79" width="0.1524" layer="91"/>
<label x="160.52" y="104.94" size="1.27" layer="95"/>
</segment>
</net>
<net name="GPS_RX" class="0">
<segment>
<pinref part="J3" gate="G$1" pin="RXD"/>
<pinref part="U3" gate="G$1" pin="GP0"/>
<wire x1="208.28" y1="96.52" x2="205.74" y2="96.52" width="0.1524" layer="91"/>
<wire x1="205.74" y1="96.52" x2="266.7" y2="96.52" width="0.1524" layer="91"/>
<wire x1="266.7" y1="96.52" x2="266.7" y2="257.81" width="0.1524" layer="91"/>
<wire x1="266.7" y1="257.81" x2="269.24" y2="257.81" width="0.1524" layer="91"/>
<label x="267.2" y="258.61" size="1.27" layer="95"/>
</segment>
</net>
<net name="GPS_TX" class="0">
<segment>
<pinref part="J3" gate="G$1" pin="TXD"/>
<pinref part="U3" gate="G$1" pin="GP1"/>
<wire x1="208.28" y1="99.06" x2="205.74" y2="99.06" width="0.1524" layer="91"/>
<wire x1="205.74" y1="99.06" x2="266.7" y2="99.06" width="0.1524" layer="91"/>
<wire x1="266.7" y1="99.06" x2="266.7" y2="255.27" width="0.1524" layer="91"/>
<wire x1="266.7" y1="255.27" x2="269.24" y2="255.27" width="0.1524" layer="91"/>
<label x="267.2" y="256.07" size="1.27" layer="95"/>
</segment>
</net>
<net name="GPS_PPS" class="0">
<segment>
<pinref part="J3" gate="G$1" pin="PPS"/>
<pinref part="U3" gate="G$1" pin="GP2"/>
<wire x1="208.28" y1="93.98" x2="205.74" y2="93.98" width="0.1524" layer="91"/>
<wire x1="205.74" y1="93.98" x2="266.7" y2="93.98" width="0.1524" layer="91"/>
<wire x1="266.7" y1="93.98" x2="266.7" y2="250.19" width="0.1524" layer="91"/>
<wire x1="266.7" y1="250.19" x2="269.24" y2="250.19" width="0.1524" layer="91"/>
<label x="267.2" y="250.99" size="1.27" layer="95"/>
</segment>
</net>
<net name="LED_A" class="0">
<segment>
<pinref part="R14" gate="G$1" pin="2"/>
<pinref part="D6" gate="G$1" pin="A"/>
<wire x1="246.38" y1="104.14" x2="248.92" y2="104.14" width="0.1524" layer="91"/>
<wire x1="248.92" y1="104.14" x2="276.86" y2="104.14" width="0.1524" layer="91"/>
<wire x1="276.86" y1="104.14" x2="274.32" y2="104.14" width="0.1524" layer="91"/>
<label x="277.36" y="104.94" size="1.27" layer="95"/>
</segment>
</net>
<net name="LED_STATUS" class="0">
<segment>
<pinref part="D6" gate="G$1" pin="K"/>
<pinref part="U3" gate="G$1" pin="GP4"/>
<wire x1="264.16" y1="104.14" x2="261.62" y2="104.14" width="0.1524" layer="91"/>
<wire x1="261.62" y1="104.14" x2="266.7" y2="104.14" width="0.1524" layer="91"/>
<wire x1="266.7" y1="104.14" x2="266.7" y2="245.11" width="0.1524" layer="91"/>
<wire x1="266.7" y1="245.11" x2="269.24" y2="245.11" width="0.1524" layer="91"/>
<label x="267.2" y="245.91" size="1.27" layer="95"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
