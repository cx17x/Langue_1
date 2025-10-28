package tree_sitter_lab1_test

import (
	"testing"

	tree_sitter "github.com/tree-sitter/go-tree-sitter"
	tree_sitter_lab1 "github.com/tree-sitter/tree-sitter-lab1/bindings/go"
)

func TestCanLoadGrammar(t *testing.T) {
	language := tree_sitter.NewLanguage(tree_sitter_lab1.Language())
	if language == nil {
		t.Errorf("Error loading Lab1 grammar")
	}
}
