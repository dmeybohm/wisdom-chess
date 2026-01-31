# Add Feature Log to CLAUDE.md

## Summary

Add "Git" and "Feature log" sections to the wisdom-chess CLAUDE.md file to
document workflow expectations for feature development and design rationales.

## Motivation

Tracking feature development and design decisions helps maintain project
knowledge over time. By storing plans in a structured `features/` directory
organized by year and month, we can:

- Keep a historical record of why features were implemented a certain way
- Consult past decisions when the code is unclear
- Track implementation progress across sessions

## Implementation

### File Modified

`CLAUDE.md`

### Sections Added

1. **Git section**: Documents expectations for working on feature branches,
   committing after significant steps, and preferring new commits over rebasing
   for fixing mistakes.

2. **Feature log section**: Describes the `features/` directory structure
   (`features/$year/$month/{$git_branch_name}.md`) and how to document
   implementation progress within feature files.

### Directory Structure

```
features/
  2026/
    01/
      add-feature-log.md  (this file)
```

The `features/` directory will grow as new features are developed.
