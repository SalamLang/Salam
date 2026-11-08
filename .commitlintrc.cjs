module.exports = {
  parserPreset: {
    parserOpts: {
      headerPattern: /^(\w*)(?:\((.*)\))?!?: (.*)$/,
      breakingHeaderPattern: /^(\w*)(?:\((.*)\))?!: (.*)$/,
      headerCorrespondence: ["type", "scope", "subject"],
      noteKeywords: ["BREAKING CHANGE", "BREAKING-CHANGE"],
    },
  },
  rules: {
    "type-empty": [2, "never"],
    "subject-empty": [2, "never"],
    "type-case": [2, "always", "lower-case"],
    "header-max-length": [2, "always", 100],
    "subject-full-stop": [2, "never", "."],
  },
};
