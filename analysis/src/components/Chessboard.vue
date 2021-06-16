<template>
  <div>
    <div ref="my_board" style="width: 400px"></div>
    <div>Depth: {{search.depth}}</div>
    <div>Move: {{search.move}}</div>
  </div>
</template>

<script>
export default {
  name: "Chessboard",
  props: {
    search: Object
  },
  mounted: function() {
    window.Chessboard(this.$refs.my_board, { position: this.search.fen });
  }
};
</script>

<style scoped>

</style>