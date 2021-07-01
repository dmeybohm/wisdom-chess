<template>
  <div>
    <div ref="my_board" style="width: 300px"></div>
  </div>
</template>

<script>
export default {
  name: "Chessboard",
  props: {
    fen: String,
    move_list: Array
  },
  mounted: function() {
    this.reload();
  },
  watch: {
    move_list: function() {
      this.reload()
    }
  },
  methods: {
    reload: function () {
      let board = window.Chessboard(this.$refs.my_board, {position: this.fen});
      let $board = window.jQuery(this.$refs.my_board);
      this.move_list.forEach(function (move) {
        if (move) {
          let squareClass = 'square-55d63'
          $board.find('.' + squareClass).removeClass('.highlight')
          let squares = move.replace(' ', 'x').split('x')
          $board.find('.square-' + squares[0]).addClass('highlight')
          $board.find('.square-' + squares[1]).addClass('highlight')
          board.move(move.replace(' ', '-'));
        }
      });
    }
  }

};
</script>

<style>
.highlight {
  box-shadow: inset 0 0 3px 3px #008000;
}
</style>