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
  methods: {
    reload: function () {
      let board = window.Chessboard(this.$refs.my_board, {position: this.fen});
      this.move_list.forEach(function (move) {
        if (move) {
          board.move(move.replace(' ', '-'));
        }
      });
    }
  }

};
</script>

<style scoped>

</style>