import XCTest
import SwiftTreeSitter
import TreeSitterLab1

final class TreeSitterLab1Tests: XCTestCase {
    func testCanLoadGrammar() throws {
        let parser = Parser()
        let language = Language(language: tree_sitter_lab1())
        XCTAssertNoThrow(try parser.setLanguage(language),
                         "Error loading Lab1 grammar")
    }
}
