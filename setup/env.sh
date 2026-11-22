# Source this in every shell:  source ~/cbv/setup/env.sh
# AMD licences (Vitis_HLS + Vivado Basic): read every .lic in ~/.Xilinx
export XILINXD_LICENSE_FILE="$HOME/.Xilinx"
# Vitis 2026.1
for d in "$HOME/tools/Xilinx" /tools/Xilinx; do
  [ -f "$d/2026.1/Vitis/settings64.sh" ] && { source "$d/2026.1/Vitis/settings64.sh"; break; }
done
# Frama-C (opam switch "cbv")
command -v opam >/dev/null && eval "$(opam env --switch=cbv --set-switch 2>/dev/null)"
# Own binaries (bambu, cvc5, z3) first; OSS CAD Suite appended last so it doesn't shadow system python
export PATH="$HOME/tools/bin:$PATH:$HOME/tools/oss-cad-suite/bin"
# Project venv
[ -f "$HOME/cbv/.venv/bin/activate" ] && source "$HOME/cbv/.venv/bin/activate"
export LC_ALL=en_US.UTF-8
