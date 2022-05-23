
module.exports = function(eleventyConfig) {
  eleventyConfig.addPassthroughCopy("verbatim/**");
};

const parse = require("csv-parse/lib/sync");

module.exports = eleventyConfig => {
  eleventyConfig.addDataExtension("csv", (contents) => {
    const records = parse(contents, {
      columns: true,
      skip_empty_lines: true,
    });
    return records;
  });
};

