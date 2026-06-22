# Project Context Notes
# =====================
# IMPORTANT: Read this file at the start of every session.

## Recurring Issue: Makefile Tabs
# CRITICAL: GNU Make REQUIRES tabs (not spaces) for recipe lines.
# When editing Makefiles with the Edit/Write tools, tabs get converted to spaces.
# ALWAYS run this after editing any Makefile:
#   sed -i 's/^        /\t/' Makefile
# Or verify with: cat -A Makefile | head -10
# Lines starting with spaces (8 spaces) = BROKEN. Lines with ^I = CORRECT.

## GitHub Token
# Stored at /home/z/my-project/.github-token
# Used for pushing to repos owned by H191box

## Repos
# - Gravity-Lab: https://github.com/H191box/Gravity-Lab.git (fixes pushed)
# - Crypt of the Pixel: https://github.com/H191box/Crypt-of-the-Pixel.git (fixes pushed)
#   Token is embedded in .git/config for this repo

## Pending Work
# - Crypt of the Pixel: needs testing after 5-bug fix (startup, input, palettes, OAM, .data)
# - Refactors: phantom-blade, iron-harvest, neural-link
# - Renames: Neural Link -> "Heartsync", Iron Harvest -> "Tactics"
