# Adding a design system

The page DSL ships with shadcn. This is the list of what another design needs.
Read `DESIGN_SYSTEMS.md` first.

1. Name. Add the value to `std/layout/values/design-values.salam` in the
   group `design_name`, with `@en` and a Persian `@fa`. Add the name to the
   `design` row of the settings table in `DESIGN_SYSTEMS.md`.
2. Tokens. Add a palette file or palette constants next to
   `std/layoutgen/design_palettes.salam`, using the same token names
   (`background`, `foreground`, `primary`, `radius`, `chart-1` and so on) so
   that `palette` and `design tokens` still apply. Colours go through
   `light-dark()` as in `DsTokensCss`.
3. Generators. Create `std/layoutgen/ds_<design>_<group>.salam` files with the
   CSS constants, all rules inside `:where()`. Do not copy the HTML builders:
   in the shadcn generator choose the CSS with
   `DsPick("<design>", SHADCN_CSS, DESIGN_CSS)`. A component that is not
   restyled needs nothing, it falls back to shadcn.
4. Schema values. If the design adds variants or sizes, add them as values with
   `@en` and `@fa` (quoted in pages), in the matching `std/layout/values` file.
5. Tests. Add `tests/en/page/design/` and `tests/fa/page/design/` pages that set
   `design = "<design>"` and `EXPECT` its CSS markers, one page for a
   component that is restyled and one for a component that falls back to
   shadcn. Keep `tests/{en,fa}/stdlib/layoutgen_design_fallback.salam` green.
6. Docs. Update `DESIGN_SYSTEMS.md` (settings table, the list of restyled
   components) and the group documents.
7. Lesson. In lesson 52 (en and fa) add the design to the settings table and a
   short example under "Other designs"; every example builds with
   `salam build --platform=web --inline`.
8. Run only the touched test sections (`page/design`, `stdlib/layoutgen`) and
   `sh website/check-quoted-sizes.sh`.
