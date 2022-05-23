
const CleanCSS = require("clean-css");
const {parse} = require("csv-parse/sync");

module.exports = function(eleventyConfig) {
  eleventyConfig.addPassthroughCopy({"verbatim/": "/"});

  eleventyConfig.addFilter("cssmin", function(code) {
    return new CleanCSS({}).minify(code).styles;
  });

  eleventyConfig.addDataExtension("csv", (contents) => {
    const records = parse(contents, {
      columns: true,
      skip_empty_lines: true,
    });
    return records;
  });
};

