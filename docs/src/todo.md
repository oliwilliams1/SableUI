# Roadmap to 1.0
### Components
- [x] Scroll view
- [x] Tab stack with component exposure/initialisation callback
- [ ] Button
- [ ] Sliders
- [ ] Input field
- [ ] Large text field
- [ ] Modal
- [ ] Checkbox
- [ ] Progress
- [ ] Dropdown
- [ ] Listbox
- [ ] Keyboard chip
- [ ] Context menu
- [ ] Link?
- [ ] Popover
- [ ] Radio
- [ ] Spinner?
- [ ] Switch
- [ ] Table (contents grid with sortable columns for complex)
- [ ] Toast
- [ ] Tooltip
- [ ] List
- [ ] Splitter element (horizontal & vertical)
- [ ] Text splitter element
- [ ] Better tree

### Bug fixes
- [ ] Draw window border
- [ ] Fix OpenGL context problems
- [ ] Triple check refresh things?
- [ ] Better/dynamic frame limiting - check "Event processing" in [glfw website](https://www.glfw.org/docs/3.3/input_guide.html)
- [ ] Fix resizing iterative thing
- [ ] Treenode leak
- [x] Cannot access root element directly in TabWithInitialiser (root=nullptr in Layout())
- [x] Overdraw with scrollview
- [x] Child component state losses
- [x] Scroll bar doesn't update on init / resize

### Graphics API
- [ ] Abstract shaders
- [ ] Abstract uniforms
- [ ] Abstract drawables
- [ ] Shaderc transpilation
- [ ] Vulkan & Metal support

### Events
- [x] Test keyboard events
- [x] Test scroll events

### Styling
- [ ] Themes
- [x] Text wrap property
- [ ] Inline text colour

### Panels
- [ ] Panel builder mode for user building layouts
- [ ] Panel editor for locking / unlocking panels at runtime
- [ ] Saving panel state across runs

### QOL
- [x] Expose titlebar content api
- [ ] Expose icon setting + program to embed api
- [ ] Allow custom fonts
- [ ] Image lazy-loading
- [ ] Image load-by-buffer
- [ ] Props?
- [ ] Simplify Panel min bounds thing
- [ ] Make bold, italic, etc macros
- [x] Click and drag scroll bar thumb

# Beyond 1.0
- [ ] Docking panels
- [ ] Web support?
- [ ] Mobile support?
- [ ] Touch support
- [ ] Expose graphics & shader api further for cross-platform custom shaders and objects
- [ ] Simple 3d renderer
- [ ] Plugin support
- [ ] External layout script + basic language